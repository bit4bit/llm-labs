# MALAGRAMATICA (DEPRECATED)

!! @LLM-ASSISTED

Migramatica resalta los errores ortograficos.

Deberia usar uno para este documento :).

Ejercicio de desarrollo usando solo OpenAI.

## USO

* `python3 -m venv venv`
* `sudo apt install gir1.2-gtksource-5`
* `source ven/bin/activate`
* `pip install -r requirements.txt`
* `export MIGRAMATICA_LLM_PROVIDER=openai MIGRAMATICA_LLM_API_KEY=xxxx python3 main.py`

## Reflexiones

- El nuevo codigo fuente es la conversacion, el modelo y el proveedor.
- Los programas ahoran son flotantes: usando la misma conversacion, con el mismo modelo y el mismo proveedor el proximo cambio puede ser diferente.
- Herramienta util para desarrollador con experiencia.
- Todo codigo generado deberia estar marcado como asistido por LLM.
