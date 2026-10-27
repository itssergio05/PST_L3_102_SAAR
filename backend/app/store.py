# Estado compartido en memoria. Se pierde al reiniciar el proceso.
lecturas: list[dict] = []
estado_led = {"color": "apagado"}
