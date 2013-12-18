require 'virtualmachine'

vm0 = VirtualMachine.new('vm0', 128)
vm0.load_asm(<<EOS)
	mov dx, 3F8h
	mov al, 40h
	mov cx, 80
loop:
	out dx, al
	dec cx
	cmp cx, 0
	jne loop
	mov dx, 3F8h
	mov al, 0ah
	out dx, al

	mov dx, fffh
	mov al, 0h
	out dx, al
EOS
vm0.run
