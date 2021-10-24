import os
import subprocess
import sys

import numpy as np

from data_handler import Configuration

TIMETABLE_COLUMNS = ["flight_AD", "flight_datetime", "flight_AL_Synchron_code", "flight_number", "flight_ID",
                     "flight_terminal_#", "flight_AP", "flight_AC_Synchron_code", "flight_AC_PAX_capacity_total",
                     "flight_PAX", "Aircraft_Stand"]


def generate_new_timetable(percentage: int, config: Configuration):
    data_dir = config.data_dir
    timetable_path = os.path.join(data_dir, f"Timetable_{percentage}.csv")
    if os.path.exists(timetable_path):
        print(f"Timetable with {percentage}% exists")
        return

    n_flights = len(config.flights)
    if percentage == 100:
        new_timetable = config.flights.copy()
    elif percentage < 100:
        to_keep = percentage * n_flights // 100
        keep_flights = np.random.choice(n_flights, to_keep, replace=False)
        new_timetable = config.flights.loc[keep_flights]
        new_timetable.reset_index(drop=True, inplace=True)
    else:
        to_add = (percentage - 100) * n_flights // 100
        added_flights = np.random.choice(n_flights, to_add, replace=True)
        added_times = np.random.choice(n_flights, to_add, replace=True)

        full_index = np.concatenate((config.flights.index.values, added_flights))
        print(full_index)
        full_times = np.concatenate((config.flights['flight_datetime'], config.flights.flight_datetime[added_times]))
        print(full_times)

        new_timetable = config.flights.loc[full_index]
        new_timetable['flight_datetime'] = full_times
        new_timetable.reset_index(drop=True, inplace=True)

    new_timetable = new_timetable[TIMETABLE_COLUMNS]
    new_timetable["Aircraft_Stand"] = np.nan
    new_timetable.to_csv(timetable_path)
    print(f"Generated timetable with {percentage}%")


def rerun_model(percentage: int, config: Configuration, time_limit: int = 5):
    data_dir = config.data_dir
    timetable_path = os.path.join(data_dir, f"Timetable_{percentage}.csv")
    solution_path = os.path.join(data_dir, f"Solution_{percentage}.csv")
    subprocess.call((
        f"cd {os.path.join('..', 'src')}; ./solution {time_limit} \\"
        f"custom \\"
        f"{os.path.join(data_dir, 'Aircraft_Classes_Private.csv')} \\"
        f"{os.path.join(data_dir, 'Aircraft_Stands_Private.csv')} \\"
        f"{os.path.join(data_dir, 'Handling_Rates_Private.csv')} \\"
        f"{os.path.join(data_dir, 'Handling_Time_Private.csv')} \\"
        f"{timetable_path} \\"
        f"{solution_path}",
        '1111',
    ), shell=True)
