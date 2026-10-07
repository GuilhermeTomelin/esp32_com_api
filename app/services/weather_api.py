import requests

from app.utils.config import WEATHER_API_KEY


BASE_URL = "http://192.168.1.50/dados"

resposta = requests.get(url)
dados = resposta.json()

print("Temperatura:", dados["temperatura"])
print("Umidade:", dados["umidade"])

def get_weather(dados: str) -> dict:
    params = {
        "key": WEATHER_API_KEY,
        "q": dados,
        "lang": "pt",
    }

    response = requests.get(
        BASE_URL,
        params=params,
        timeout=10,
    )

    if response.status_code != 200:
        raise RuntimeError(
            f"Erro ao consultar a API. "
            f"Status: {response.status_code}"
        )

    return response.json()