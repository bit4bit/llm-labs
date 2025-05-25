Gem::Specification.new do |s|
  s.name        = 'llmed'
  s.version     = '0.1.0'
  s.licenses    = ['GPLv3']
  s.summary     = "LLM editor"
  s.description = "LLM editor"
  s.authors     = ["Jovany Leandro G.C"]
  s.email       = 'bit4bit@riseup.net'
  s.files       = ["lib/llmed.rb"]
  s.homepage    = 'https://rubygems.org/gems/example'
  s.metadata    = { "source_code_uri" => "https://github.com/bit4bit/llm-labs" }
  
  s.bindir = 'exe'
  s.executables << 'llmed'

  s.add_dependency "langchainrb", "~> 0.19.5"
  s.add_dependency "ruby-openai", "~> 8.1"

  s.add_development_dependency 'rspec'
  s.add_development_dependency 'rubocop'
end
