require 'virtualmachine'
require 'time'

if ARGV.size < 1
	puts "date.rb [repeat]"
	exit 1
end

puts "[args]"
puts "repeat:#{ARGV[0]}"
puts

vm0 = VirtualMachine.new('vm0', 128)
vm0.rcx = ARGV[0].to_i
vm0.load_asm(<<EOS)
	mov al, 0
	mov dx, 3F8h
loop:
	out dx, al
	dec cx
	cmp cx, 0
	jne loop
	mov al, 0Ah
	out dx, al

	mov dx, 64h 
	mov al, 0
	out dx, al
EOS

puts "[registers]"
puts "rax:#{vm0.rax.to_s(16)}"
puts "rbx:#{vm0.rbx.to_s(16)}"
puts "rcx:#{vm0.rcx.to_s(16)}"
puts "rdx:#{vm0.rdx.to_s(16)}"
puts "rip:#{vm0.rip.to_s(16)}"
puts

puts "[run vm]"
rip = vm0.rip
while true
	vm0.run(rip)

	if vm0.vme_exitcode == VirtualMachine::EXITCODE_BOGUS
		rip = vm0.vme_rip
	elsif vm0.vme_exitcode == VirtualMachine::EXITCODE_INOUT && vm0.vme_port == 0x3F8 && vm0.vme_in == 0
		p Time.now
		rip = vm0.vme_rip + vm0.vme_inst_length
	elsif vm0.vme_exitcode == VirtualMachine::EXITCODE_INOUT && vm0.vme_port == 0x64
		break
	else
		puts "Not handled VMExit code: #{vm0.vme_exitcode}"
		break
	end
end
puts
puts "[registers]"
puts "rax:#{vm0.rax.to_s(16)}"
puts "rbx:#{vm0.rbx.to_s(16)}"
puts "rcx:#{vm0.rcx.to_s(16)}"
puts "rdx:#{vm0.rdx.to_s(16)}"
puts "rip:#{vm0.rip.to_s(16)}"
