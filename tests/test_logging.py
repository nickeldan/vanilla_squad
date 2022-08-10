import pathlib
import re
import subprocess
import sys
import time

import pytest

THIS_DIR = pathlib.Path(__file__).parent

FORMAT_EXECUTABLE = THIS_DIR / "format"

DATE_PATTERN = re.compile(r"[A-Z][a-z]{2} [A-Z][a-z]{2} \d{2} \d{2}:\d{2}:\d{2} \d{4}$")


def test_format_message():
    p = subprocess.run([FORMAT_EXECUTABLE, "%M"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert p.stdout == "Check"


def test_format_pid():
    p = subprocess.Popen([FORMAT_EXECUTABLE, "%p"], stdout=subprocess.PIPE, encoding="utf-8")
    stdout, _ = p.communicate()
    assert p.returncode == 0
    assert stdout == str(p.pid)


def test_format_tid():
    if not sys.platform.startswith("linux"):
        pytest.skip()

    p = subprocess.Popen([FORMAT_EXECUTABLE, "%T"], stdout=subprocess.PIPE, encoding="utf-8")
    stdout, _ = p.communicate()
    assert p.returncode == 0
    assert stdout == str(p.pid)


def test_format_level():
    p = subprocess.run([FORMAT_EXECUTABLE, "%L"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert p.stdout == "INFO"


def test_format_spacing():
    p = subprocess.run([FORMAT_EXECUTABLE, "%_"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert p.stdout == " " * 4


def test_format_epoch_time():
    p = subprocess.run([FORMAT_EXECUTABLE, "%u"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert abs(time.time() - int(p.stdout)) < 2


def test_format_pretty_time():
    p = subprocess.run([FORMAT_EXECUTABLE, "%t"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert DATE_PATTERN.match(p.stdout)


def test_format_hour():
    p = subprocess.run([FORMAT_EXECUTABLE, "%h"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert int(p.stdout) == time.localtime().tm_hour


def test_format_minute():
    p = subprocess.run([FORMAT_EXECUTABLE, "%m"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert int(p.stdout) == time.localtime().tm_min


def test_format_second():
    p = subprocess.run([FORMAT_EXECUTABLE, "%s"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert abs(time.localtime().tm_sec - int(p.stdout)) < 2


def test_format_file_name():
    p = subprocess.run([FORMAT_EXECUTABLE, "%F"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert p.stdout == "test_format.c"


def test_format_function_name():
    p = subprocess.run([FORMAT_EXECUTABLE, "%f"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert p.stdout == "main"


def test_format_line_number():
    p = subprocess.run([FORMAT_EXECUTABLE, "%l"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert int(p.stdout) > 0


def test_format_percent():
    p = subprocess.run([FORMAT_EXECUTABLE, "%%"], stdout=subprocess.PIPE, encoding="utf-8")
    assert p.returncode == 0
    assert p.stdout == "%"
