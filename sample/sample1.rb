require 'virtualmachine'

vm0 = VirtualMachine.new('vm0', 128)
vm0.load_sam(<<EOS)
	mov dx, 3F8h
	mov al, 2Ah
loop:
	out dx, al
	jmp loop
EOS
vm0.run
