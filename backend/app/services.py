from . import store
from .schemas import LecturaEntrada, ComandoLED

def guardar_lectura(datos: LecturaEntrada) -> dict:
    lectura = datos.model_dump()
    store.lecturas.append(lectura)
    # Conservamos como máximo las últimas 100 lecturas recibidas.
    del store.lecturas[:-100]
    return {"lectura": lectura, "led": store.estado_led["color"]}

def listar_lecturas() -> list[dict]:
    return store.lecturas

def consultar_led() -> dict:
    return store.estado_led

def cambiar_led(comando: ComandoLED) -> dict:
    store.estado_led["color"] = comando.color
    return store.estado_led

def calcular_promedio(n: int) -> dict | None:
    if len(store.lecturas) == 0:
        return None
        
    ultimas = store.lecturas[-n:]
    cantidad = len(ultimas)
    
    suma_temp = sum(lectura["temperatura"] for lectura in ultimas)
    suma_hum = sum(lectura["humedad"] for lectura in ultimas)
    
    return {
        "cantidad": cantidad,
        "temperatura_promedio": round(suma_temp / cantidad, 2),
        "humedad_promedio": round(suma_hum / cantidad, 2)
    }