import os
try:
    _SUMO_HOME = os.environ["SUMO_HOME"]
except KeyError:
    raise EnvironmentError("Please declare environment variable 'SUMO_HOME'")
os.add_dll_directory(os.path.join(_SUMO_HOME, "bin"))
from .PyV2Sim import V2SimError, V2SimInterface