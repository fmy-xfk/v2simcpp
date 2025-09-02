import time
from pathlib import Path
from Cv2sim import *

case = Path("./case").absolute()
outdir = case / "output"
outdir.mkdir(exist_ok=True)

app = V2SimInterface(0, 172800, 10,
        str(case / "test.net.xml"),
        str(case / "test.veh.xml"),
        str(case / "test.fcs.xml"),
        str(case / "test.scs.xml"),
        str(outdir),
        log_fcs=True,
        log_scs=True,
        log_ev=False
    )

app.Start()

startT = time.time()
lastT = 0

while app.getTime() < app.getEndTime():
    if time.time() - lastT > 1:
        lastT = time.time()
        print("\r", app.getTime(), "/", app.getEndTime(), f"| Elapsed Time: {time.time() - startT:.1f}s", end = "")
    app.Step()

app.Stop()
