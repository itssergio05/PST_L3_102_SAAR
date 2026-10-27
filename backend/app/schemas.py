from typing import Literal
from pydantic import BaseModel, Field

class LecturaEntrada(BaseModel):
    dispositivo: str = Field(min_length=1, max_length=40)
    temperatura: float = Field(ge=0, le=50, allow_inf_nan=False)
    humedad: float = Field(ge=0, le=100, allow_inf_nan=False)

class ComandoLED(BaseModel):
    color: Literal["rojo", "verde", "azul", "apagado"]
