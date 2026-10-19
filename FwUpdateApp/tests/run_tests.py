"""SDK-free FwUpdateApp tests. Run from any directory with Python 3 and GCC."""
from pathlib import Path
import subprocess
import tempfile

LIB = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='fw-update-app-') as directory:
    output = str(Path(directory) / 'test_fw_update_app')
    subprocess.run([
        'gcc', '-std=c99', '-Wall', '-Wextra', '-Werror',
        str(LIB / 'tests/test_fw_update_app.c'),
        str(LIB / 'src/fw_update_app.c'), '-o', output
    ], check=True)
    subprocess.run([output], check=True)
