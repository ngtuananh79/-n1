# server.py — bổ sung phục vụ dashboard tại "/"
from fastapi import FastAPI, Header, HTTPException, Query
from fastapi.responses import FileResponse
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field
from typing import Optional
import sqlite3, time, os

app = FastAPI(title="LAN IoT Server with SQLite")

# CORS (giúp mở dashboard từ trình duyệt máy khác trong LAN)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],      # có thể siết chặt lại sau
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

API_KEY = "changeme"
DB_PATH = os.path.join(os.path.dirname(__file__), "data.db")
DASHBOARD_PATH = os.path.join(os.path.dirname(__file__), "dashboard.html")

# ==== SQLite ====
conn = sqlite3.connect(DB_PATH, check_same_thread=False)
cur  = conn.cursor()
cur.execute("""
CREATE TABLE IF NOT EXISTS readings (
  id        TEXT,
  value     REAL,
  unit      TEXT,
  humidity  REAL,
  ts        INTEGER,
  recv_ts   INTEGER
)
""")
cur.execute("CREATE INDEX IF NOT EXISTS idx_recv_ts ON readings (recv_ts DESC)")
conn.commit()

# ==== Model ====
class SensorPayload(BaseModel):
    id: str = Field(..., description="Mã thiết bị")
    value: float = Field(..., description="Giá trị đo chính")
    unit: Optional[str] = Field(default="C")
    humidity: Optional[float] = None
    ts: Optional[int] = None

# ==== Root: trả về dashboard.html ====
@app.get("/", include_in_schema=False)
def dashboard():
    # Nếu cần: kiểm tra file tồn tại
    if not os.path.exists(DASHBOARD_PATH):
        return {"error": "dashboard.html not found — please put it into the server/ folder"}
    return FileResponse(DASHBOARD_PATH, media_type="text/html; charset=utf-8")

# ==== Health ====
@app.get("/ping")
def ping():
    return {"status": "ok"}

# ==== Ingest ====
@app.post("/sensor")
def ingest(s: SensorPayload, x_api_key: Optional[str] = Header(default=None)):
    if API_KEY and x_api_key != API_KEY:
        raise HTTPException(status_code=401, detail="Invalid API key")
    recv_ts = int(time.time())
    cur.execute(
        "INSERT INTO readings (id, value, unit, humidity, ts, recv_ts) VALUES (?,?,?,?,?,?)",
        (s.id, float(s.value), s.unit, None if s.humidity is None else float(s.humidity), s.ts, recv_ts)
    )
    conn.commit()
    return {"ok": True, "recv_ts": recv_ts}

# ==== Latest ====
@app.get("/latest")
def latest(n: int = Query(10, ge=1, le=500)):
    rows = cur.execute(
        "SELECT id, value, unit, humidity, ts, recv_ts FROM readings ORDER BY recv_ts DESC LIMIT ?",
        (n,)
    ).fetchall()
    return [
        {"id": r[0], "value": r[1], "unit": r[2], "humidity": r[3], "ts": r[4], "recv_ts": r[5]}
        for r in rows
    ]

# ==== Theo thiết bị (tuỳ chọn) ====
@app.get("/device/{device_id}")
def by_device(device_id: str, n: int = Query(20, ge=1, le=1000)):
    rows = cur.execute(
        "SELECT id, value, unit, humidity, ts, recv_ts FROM readings WHERE id = ? ORDER BY recv_ts DESC LIMIT ?",
        (device_id, n)
    ).fetchall()
    return [
        {"id": r[0], "value": r[1], "unit": r[2], "humidity": r[3], "ts": r[4], "recv_ts": r[5]}
        for r in rows
    ]
