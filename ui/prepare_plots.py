import os

import numpy as np
import pandas as pd


class Configuration:
    def __init__(self, data_dir):
        self.data_dir = data_dir
        self.aircraft_classes = pd.read_csv(os.path.join(data_dir, "Aircraft_Classes_Private.csv"), index_col=0,
                                            squeeze=True)
        self.stands = pd.read_csv(os.path.join(data_dir, "Aircraft_Stands_Private.csv"), index_col=0)
        self.handling_rates = pd.read_csv(os.path.join(data_dir, "Handling_Rates_Private.csv"), index_col=0,
                                          squeeze=True)
        self.handling_times = pd.read_csv(os.path.join(data_dir, "Handling_Time_Private.csv"), index_col=0,
                                          squeeze=True)
        self.flights = pd.read_csv(os.path.join(data_dir, "Timetable_private.csv"), index_col=0)

        self.bus_cost = self.handling_rates['Bus_Cost_per_Minute']
        self.away_ac_stand_cost = self.handling_rates['Away_Aircraft_Stand_Cost_per_Minute']
        self.jetbridge_ac_stand_cost = self.handling_rates['JetBridge_Aircraft_Stand_Cost_per_Minute']
        self.taxiing_cost = self.handling_rates['Aircraft_Taxiing_Cost_per_Minute']

        self.bus_capacity = 80

        self.normalize_terminals()
        self.normalize_flights()

    def normalize_terminals(self):
        self.stands.Terminal = [-1 if pd.isna(t) else int(t) for t in self.stands.Terminal]

    def normalize_flights(self):
        def get_class_from_capacity(flight_capacity):
            for ac_class, capacity in self.aircraft_classes.iteritems():
                if flight_capacity <= capacity:
                    return ac_class
            raise ValueError(f"Incorrect flight capacity {flight_capacity}")

        self.flights["ac_class"] = [get_class_from_capacity(cap) for cap in self.flights.flight_AC_PAX_capacity_total]

    def ac_classes(self):
        return self.aircraft_classes.index.tolist()

feature_ids = [
    'total_cost', 'taxiing_cost', 'bus_cost', 'parking_cost', 'handling_time', 'taxiing_time', 'use_jet_bridge',
    'n_buses',
    'n_passengers'
]


def calculate_features(flight_id: int, config: Configuration, flights_mapping: dict):
    flight = config.flights.loc[flight_id]
    stand = config.stands.loc[flights_mapping[flight_id]]
    taxiing_cost = stand.Taxiing_Time * config.taxiing_cost
    if flight.flight_AD == 'A':
        jet_bridge = stand.JetBridge_on_Arrival
    elif flight.flight_AD == 'D':
        jet_bridge = stand.JetBridge_on_Departure
    else:
        raise ValueError(f'Incorrect flight_AD for flight {flight_id}')

    if jet_bridge == 'N':
        parking_cost = config.away_ac_stand_cost
    else:
        if jet_bridge not in ['I', 'D']:
            raise ValueError(f'Incorrect jet_bridge for stand {flights_mapping[flight_id]}')
        parking_cost = config.jetbridge_ac_stand_cost

    stand_terminal = stand.Terminal
    flight_terminal = flight["flight_terminal_#"]
    if jet_bridge == flight.flight_ID and flight_terminal == stand_terminal:
        use_jet_bridge = 1
        handling_time = config.handling_times.JetBridge_Handling_Time[flight["ac_class"]]
        n_buses = 0
        bus_cost = 0
    else:
        use_jet_bridge = 0
        handling_time = config.handling_times.Away_Handling_Time[flight["ac_class"]]
        n_buses = (flight.flight_PAX + config.bus_capacity - 1) // config.bus_capacity
        bus_cost = n_buses * config.bus_cost * stand[str(flight_terminal)]

    parking_cost = handling_time * parking_cost

    return np.array([
        taxiing_cost + bus_cost + parking_cost,
        taxiing_cost, bus_cost,
        parking_cost,
        handling_time,
        stand.Taxiing_Time,
        use_jet_bridge,
        n_buses,
        flight.flight_PAX
    ])


def get_config():
    data_dir = os.path.join("..", "data", "private")
    config = Configuration(data_dir)
    flights_mapping = pd.read_csv(os.path.join(data_dir, "Solution_Private_1994060.csv"), index_col=0).Aircraft_Stand
    features = np.array([calculate_features(flight_id, config, flights_mapping) for flight_id in config.flights.index])

    for i, feature in enumerate(feature_ids):
        config.flights[feature] = features[:, i]

    return config
