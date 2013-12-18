/*-
 * Copyright (c) 2011 NetApp, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NETAPP, INC ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NETAPP, INC OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

/*-
 * Copyright (c) 2011 Google, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");
#include <sys/param.h>
#include <sys/stat.h>
#include <machine/specialreg.h>
#include <machine/vmm.h>
#include <x86/segments.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vmmapi.h>

#include <ruby.h>

#define MB      (1024 * 1024UL)

#define MSR_EFER        	0xc0000080
#define CR4_PAE         	0x00000020
#define CR4_PSE         	0x00000010
#define CR0_PG          	0x80000000
#define	CR0_PE			0x00000001
#define	CR0_NE			0x00000020

#define PG_V			0x001
#define PG_RW			0x002
#define PG_U			0x004
#define PG_PS			0x080

#define ADDR_PT4 		0x2000
#define ADDR_PT3 		0x3000
#define ADDR_PT2 		0x4000
#define ADDR_GDT 		0x5000
#define ADDR_STACK 		0x6000
#define ADDR_ENTRY 		0x10000

#define	DESC_UNUSABLE		0x00010000

#define	GUEST_NULL_SEL		0
#define	GUEST_CODE_SEL		1
#define	GUEST_DATA_SEL		2
#define	GUEST_GDTR_LIMIT	(3 * 8 - 1)

VALUE rb_cVirtualMachine;
VALUE rb_cVMCtx;

VALUE vmctx_free(VALUE self)
{
	return Qnil;
}

int _vm_create(const char *name)
{
	int error = vm_create(name);
	if (error)
		rb_raise(rb_eException, "vm_create failed");
	return error;
}

struct vmctx *_vm_open(const char *name)
{
	struct vmctx *ctx = vm_open(name);
	if (!ctx)
		rb_raise(rb_eException, "vm_open failed");
	return ctx;
}

int _vm_setup_memory(struct vmctx *ctx, size_t len, enum vm_mmap_style s)
{
	int error = vm_setup_memory(ctx, len, s);
	if (error)
		rb_raise(rb_eException, "vm_setup_memory failed error=%d",
			error);
	return error;
}

void *_vm_map_gpa(struct vmctx *ctx, vm_paddr_t gaddr, size_t len)
{
	void *paddr = vm_map_gpa(ctx, gaddr, len);
	if (!paddr)
		rb_raise(rb_eException, "vm_map_gpa failed");
	return paddr;
}

int _vm_set_desc(struct vmctx *ctx, int vcpu, int reg,
	uint64_t base, uint32_t limit, uint32_t access)
{
	int error = vm_set_desc(ctx, vcpu, reg, base, limit, access);
	if (error)
		rb_raise(rb_eException, "vm_set_desc failed error=%d",
			error);
	return error;
}

int _vm_set_register(struct vmctx *ctx, int vcpu, int reg, uint64_t val)
{
	int error = vm_set_register(ctx, vcpu, reg, val);
	if (error)
		rb_raise(rb_eException, "vm_set_register failed error=%d",
			error);
	return error;
}

int _vm_get_register(struct vmctx *ctx, int vcpu, int reg, uint64_t *retval)
{
	int error = vm_get_register(ctx, vcpu, reg, retval);
	if (error)
		rb_raise(rb_eException, "vm_get_register failed error=%d",
			error);
	return error;
}

VALUE virtualmachine_initialize(VALUE self, VALUE vmname, VALUE memsize)
{
	VALUE vctx;
	struct vmctx *ctx;
	uint64_t *gdt, *pt4, *pt3, *pt2;
	int i, err;

	_vm_create(StringValuePtr(vmname));
	ctx = _vm_open(StringValuePtr(vmname));
	vctx = Data_Wrap_Struct(rb_cVMCtx, NULL, vmctx_free, ctx);
	rb_iv_set(self, "@ctx", vctx);
	rb_iv_set(self, "@vmname", vmname);
	rb_iv_set(self, "@memsize", memsize);
	_vm_setup_memory(ctx, FIX2INT(memsize) * MB, VM_MMAP_ALL);

	pt4 = _vm_map_gpa(ctx, ADDR_PT4, sizeof(uint64_t) * 512);
	pt3 = _vm_map_gpa(ctx, ADDR_PT3, sizeof(uint64_t) * 512);
	pt2 = _vm_map_gpa(ctx, ADDR_PT2, sizeof(uint64_t) * 512);
	gdt = _vm_map_gpa(ctx, ADDR_GDT, sizeof(uint64_t) * 3);

	bzero(pt4, PAGE_SIZE);
	bzero(pt3, PAGE_SIZE);
	bzero(pt2, PAGE_SIZE);

	for (i = 0; i < 512; i++) {
		pt4[i] = (uint64_t)ADDR_PT3;
		pt4[i] |= PG_V | PG_RW | PG_U;
		pt3[i] = (uint64_t)ADDR_PT2;
		pt3[i] |= PG_V | PG_RW | PG_U;
		pt2[i] = i * (2 * 1024 * 1024);
		pt2[i] |= PG_V | PG_RW | PG_PS | PG_U;
	}

	gdt[GUEST_NULL_SEL] = 0;
	gdt[GUEST_CODE_SEL] = 0x0020980000000000;
	gdt[GUEST_DATA_SEL] = 0x0000900000000000;

	_vm_set_desc(ctx, 0, VM_REG_GUEST_CS, 0, 0, 0x0000209B);
	_vm_set_desc(ctx, 0, VM_REG_GUEST_DS, 0, 0, 0x00000093);
	_vm_set_desc(ctx, 0, VM_REG_GUEST_ES, 0, 0, 0x00000093);
	_vm_set_desc(ctx, 0, VM_REG_GUEST_FS, 0, 0, 0x00000093);
	_vm_set_desc(ctx, 0, VM_REG_GUEST_GS, 0, 0, 0x00000093);
	_vm_set_desc(ctx, 0, VM_REG_GUEST_SS, 0, 0, 0x00000093);
	_vm_set_desc(ctx, 0, VM_REG_GUEST_TR, 0, 0, 0x0000008b);
	_vm_set_desc(ctx, 0, VM_REG_GUEST_LDTR, 0, 0, DESC_UNUSABLE);
	_vm_set_desc(ctx, 0, VM_REG_GUEST_GDTR, ADDR_GDT, GUEST_GDTR_LIMIT, 0);

	_vm_set_register(ctx, 0, VM_REG_GUEST_CS, GSEL(GUEST_CODE_SEL, SEL_KPL));
	_vm_set_register(ctx, 0, VM_REG_GUEST_DS, GSEL(GUEST_DATA_SEL, SEL_KPL));
	_vm_set_register(ctx, 0, VM_REG_GUEST_ES, GSEL(GUEST_DATA_SEL, SEL_KPL));
	_vm_set_register(ctx, 0, VM_REG_GUEST_FS, GSEL(GUEST_DATA_SEL, SEL_KPL));
	_vm_set_register(ctx, 0, VM_REG_GUEST_GS, GSEL(GUEST_DATA_SEL, SEL_KPL));
	_vm_set_register(ctx, 0, VM_REG_GUEST_SS, GSEL(GUEST_DATA_SEL, SEL_KPL));
	_vm_set_register(ctx, 0, VM_REG_GUEST_TR, 0);
	_vm_set_register(ctx, 0, VM_REG_GUEST_LDTR, 0);

	_vm_set_register(ctx, 0, VM_REG_GUEST_CR0, CR0_PG | CR0_PE | CR0_NE);
	_vm_set_register(ctx, 0, VM_REG_GUEST_CR3, ADDR_PT4);
	_vm_set_register(ctx, 0, VM_REG_GUEST_CR4, CR4_PAE | CR4_VMXE);
	_vm_set_register(ctx, 0, VM_REG_GUEST_EFER, EFER_LMA | EFER_LME);
	_vm_set_register(ctx, 0, VM_REG_GUEST_RFLAGS, 0x2);
	_vm_set_register(ctx, 0, VM_REG_GUEST_RSP, ADDR_STACK);

	return self;
}

VALUE virtualmachine_load_binary(VALUE self, VALUE program)
{
	VALUE vctx = rb_iv_get(self, "@ctx");
	struct vmctx *ctx;
	unsigned char *entry;

	Data_Get_Struct(vctx, struct vmctx, ctx);
	if (!ctx)
		rb_bug("ctx is null");
	entry = vm_map_gpa(ctx, ADDR_ENTRY, RSTRING_LEN(program));
	memcpy(entry, StringValuePtr(program), RSTRING_LEN(program));
	vm_set_register(ctx, 0, VM_REG_GUEST_RIP, ADDR_ENTRY);
	return Qnil;
}

VALUE virtualmachine_destroy(VALUE self)
{
	VALUE vctx = rb_iv_get(self, "@ctx");
	struct vmctx *ctx;
	Data_Get_Struct(vctx, struct vmctx, ctx);
	if (!ctx)
		rb_bug("ctx is null");
	vm_destroy(ctx);
	return Qnil;
}

void Init_virtualmachine(void)
{
	rb_cVirtualMachine = rb_define_class("VirtualMachine", rb_cObject);
	rb_cVMCtx = rb_define_class("VMCtx", rb_cObject);
	rb_define_method(rb_cVirtualMachine, "initialize", virtualmachine_initialize, 2);
	rb_define_method(rb_cVirtualMachine, "load_binary", virtualmachine_load_binary, 1);
	rb_define_method(rb_cVirtualMachine, "destroy", virtualmachine_destroy, 0);
}
