# VirtualMachine

virtualmachine allows to run your assembly code on a VMM instantly, using Ruby.

## Installation

Add this line to your application's Gemfile:

    gem 'virtualmachine'

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install virtualmachine

## Usage

Here's a sample code.
This will create a VM named 'vm0' with 128MB memory, executes an assembly code witch passed to load_asm().

    require 'virtualmachine'
    vm0 = VirtualMachine.new('vm0', 128)
    vm0.load_asm(<<EOS)
    	mov dx, 3F8h
    	mov al, 2Ah
    loop:
    	out dx, al
    	jmp loop
    EOS
    vm0.run

## Limitations/TODO
- Currently, virtualmachine only supports FreeBSD-10 or later, because it built on BHyVe. Need to support Linux KVM.
- Page table, GDT, control registers are pre-initialized, hardcoded. Need to find better way to do it.
- No way to read/write guest registers and guest memory area. Need more APIs.
- Due to it's hardcoded, you can't choice 16bit/32bit mode. It always runs in 64bit mode.
- Because BHyVe doesn't have a BIOS, you have no way to call it.

## Contributing

1. Fork it
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create new Pull Request
