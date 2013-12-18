# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)

Gem::Specification.new do |spec|
  spec.name          = "virtualmachine"
  spec.version       = "0.0.3"
  spec.authors       = ["Takuya ASADA"]
  spec.email         = ["syuu@dokukino.com"]
  spec.extensions    = ["ext/virtualmachine/extconf.rb"]
  spec.description   = "Run your asm code on VMM instantly, using ruby"
  spec.summary       = "Run your asm code on VMM instantly, using ruby"
  spec.homepage      = "https://github.com/syuu1228/ruby-virtualmachine"
  spec.license       = "BSDL"

  spec.files         = `git ls-files`.split($/)
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.3"
  spec.add_development_dependency "rake"

  spec.add_dependency "metasm", ">= 0"
end
