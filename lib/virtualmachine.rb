require 'virtualmachine.so'
require 'metasm'

class VirtualMachine
	EXITCODE_INOUT = 0
	EXITCODE_VMX = 1
	EXITCODE_BOGUS = 2
	def load_asm(program)
		edata = Metasm::Shellcode.assemble(Metasm::X64.new, program).encoded
		load_binary(edata.data)
	end
end
