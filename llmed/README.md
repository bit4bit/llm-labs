# LLMED

Single LLM Code editor.

Premises:
* Messages -> Source Code
* Output LLM -> Application

So we need a better way to express the intention of the program.

```ruby
application "hola mundo ruby", language: 'ruby', output_file: "holamundo-output.rb" do
  context "main" do
    <<-LLM
        Codigo ruby que imprima 'hola mundo'.
    LLM
  end
end

application "hola mundo python", language: 'python', output_file: "holamundo-output.py" do
  context "main" do
    <<-LLM
        Codigo python que imprima 'hola mundo'.
    LLM
  end
end
```

# Installation

* `bundle3.1 install --path vendor/`

# Usage

`OPENAI_API_KEY=xxx rake llmed[examples/tictactoe.rb]`
