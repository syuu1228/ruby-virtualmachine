require 'virtualmachine'

if ARGV.size < 2
	puts "sample1.rb [char] [repeat]"
	exit 1
end

puts "[args]"
puts "char:#{ARGV[0]}"
puts "repeat:#{ARGV[1]}"
puts

vm0 = VirtualMachine.new('vm0', 128)
vm0.rax = ARGV[0].bytes.to_a[0]
vm0.rcx = ARGV[1].to_i
vm0.load_asm(<<EOS)
	mov dx, 3F8h
loop:
	out dx, al
	dec cx
	cmp cx, 0
	jne loop
	mov al, 0ah
	out dx, al

	mov dx, 10h 
	mov al, 0h
	out dx, al
EOS

puts "[registers]"
puts "rax:#{vm0.rax}"
puts "rbx:#{vm0.rbx}"
puts "rcx:#{vm0.rcx}"
puts "rdx:#{vm0.rdx}"
puts "rip:#{vm0.rip}"
puts

puts "[run vm]"
vm0.run

puts
puts "[registers]"
puts "rax:#{vm0.rax}"
puts "rbx:#{vm0.rbx}"
puts "rcx:#{vm0.rcx}"
puts "rdx:#{vm0.rdx}"
puts "rip:#{vm0.rip}"
