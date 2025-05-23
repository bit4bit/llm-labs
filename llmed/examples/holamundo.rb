set_llm provider: :openai, api_key: ENV['OPENAI_API_KEY'], model: 'gpt-4o'

application "hola mundo ruby", language: 'ruby', output_file: "holamundo-output.rb" do
  context "main" do
    <<-LLM
        Codigo que imprima 'hola mundo'.
    LLM
  end
end

application "hola mundo python", language: 'python', output_file: "holamundo-output.py" do
  context "main" do
    llm <<-LLM
        Codigo que imprima 'hola mundo'.
    LLM
  end
end
