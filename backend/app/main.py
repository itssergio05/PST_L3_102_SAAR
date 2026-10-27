from fastapi import FastAPI
from .routes import router

app = FastAPI(title="Lab 3 Backend IoT", version="1.0")
app.include_router(router)
