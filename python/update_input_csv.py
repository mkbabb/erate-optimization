from __future__ import annotations

import pathlib
import time
import tomllib
from typing import *

import pandas as pd
from googleapiutils2 import Drive, Sheets, get_oauth2_creds
from loguru import logger


def sync_sheet_csv(
    file_path: pathlib.Path,
    file_id: str,
    range_name: str,
    sheets: Sheets,
    interval: float = 1.0,
):
    while True:
        try:
            df = sheets.to_frame(
                sheets.values(
                    spreadsheet_id=file_id,
                    range_name=range_name,
                )
            )
            df.to_csv(file_path, index=False)
        except Exception as e:
            logger.error(e)
            pass

        time.sleep(interval)


drive = Drive()
sheets = Sheets()

config_path = pathlib.Path("./config.toml")
config = tomllib.loads(
    config_path.read_text(),
)

optimization_sheet_id = config["google"]["optimization_sheet_id"]
input_range_name = config["google"]["input_range_name"]

file_path = pathlib.Path("./data/input.csv")

sync_sheet_csv(
    file_path=file_path,
    file_id=optimization_sheet_id,
    range_name=input_range_name,
    sheets=sheets,
)
