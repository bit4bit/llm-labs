# langchain_multi_provider.py
# Don't waste your time reading this it is just AI trash
# @LLM-ASSISTED
import os
import re
import json
from langchain.prompts import PromptTemplate
from langchain.schema.runnable import Runnable
from langchain.chat_models import ChatOpenAI
from langchain_google_genai import ChatGoogleGenerativeAI


class MultiProviderLLM:
    def __init__(self, provider: str, api_key: str):
        self.provider = provider.lower()
        self.api_key = api_key
        self.model = self._select_model()
        self.llm = self._init_llm()

    def _select_model(self) -> str:
        if self.provider == "openai":
            return "gpt-4o"
        elif self.provider == "gemini":
            return "gemini-2.0-flash"
        else:
            raise ValueError(f"Proveedor '{self.provider}' no soportado.")

    def _init_llm(self) -> Runnable:
        if self.provider == "openai":
            os.environ["OPENAI_API_KEY"] = self.api_key
            return ChatOpenAI(model_name=self.model)
        elif self.provider == "gemini":
            os.environ["GOOGLE_API_KEY"] = self.api_key
            return ChatGoogleGenerativeAI(model=self.model)

    def run(self, prompt_file: str, user_input: str, language: str) -> dict:
        with open(prompt_file, "r", encoding="utf-8") as f:
            prompt_content = f.read()

        prompt = PromptTemplate.from_template(prompt_content)
        chain = prompt | self.llm
        response = chain.invoke({
            "input": user_input,
            "language": language
        })

        text = response.content.strip()

        json_match = re.search(r"```json\s*(\{.*?\})\s*```", text, re.DOTALL | re.IGNORECASE)
        if json_match:
            json_text = json_match.group(1)
        else:
            json_text = re.sub(r'^\s*json\s*', '', text, flags=re.IGNORECASE)

        return json.loads(json_text)
