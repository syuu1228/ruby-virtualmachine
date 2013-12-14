require "mkmf"

dir_config('vmmapi', '/usr/include', '/usr/lib')
if have_header('vmmapi.h') and have_library('util', 'expand_number', 'util.h') and have_library('vmmapi', 'vm_create', 'vmmapi.h')
        create_makefile('virtualmachine')
end
