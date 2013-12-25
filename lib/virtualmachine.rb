require 'virtualmachine.so'
require 'metasm'

class VirtualMachine
	def load_asm(program)
		edata = Metasm::Shellcode.assemble(Metasm::X64.new, program).encoded
		load_binary(edata.data)
	end

	def run
		system("/usr/sbin/bhyve -H -c 1 -m #{@memsize} -s 0:0,hostbridge -S 31,uart,stdio -e #{@vmname} 2> /dev/null")
	end
end
