from collections import defaultdict, Counter

import dash
import dash_bootstrap_components as dbc
import dash_core_components as dcc
import dash_html_components as html
import plotly.express as px
import plotly.graph_objects as go
from dash.dependencies import Input, Output
from plotly.subplots import make_subplots

from data_handler import get_config, Configuration
from traffic_emulator import generate_new_timetable, rerun_model

PLOTLY_LOGO = "https://images.plot.ly/logo/new-branding/plotly-logomark.png"

default_config = get_config()

NAVBAR = dbc.Navbar(
    children=[
        html.A(
            # Use row and col to control vertical alignment of logo / brand
            dbc.Row(
                [
                    dbc.Col(html.Img(src=PLOTLY_LOGO, height="30px")),
                    dbc.Col(
                        dbc.NavbarBrand("Report for SVO data case", className="ml-2")
                    ),
                ],
                align="center",
                no_gutters=True,
            ),
            href="https://github.com/Nikitosh/Vehicles_SVO_Case",
        )
    ],
    color="dark",
    dark=True,
    sticky="top",
)

LEFT_COLUMN = dbc.Jumbotron(
    [
        html.H4(children="Выбор загруженности аэропорта для анализа", className="display-5"),
        html.Hr(className="my-2"),
        html.Label("Выберите долю загрузки аэропорта относительно пикового дня", className="lead"),
        html.P(
            "(При загрузке меньше 100% часть рейсов будут удалены. При загрузке больше 100% будут сгенерированы новые рейсы на основе имеющихся.)",
            style={"fontSize": 10, "font-weight": "lighter"},
        ),
        dcc.Slider(
            id="n-selection-slider",
            min=5,
            max=300,
            step=5,
            marks={
                0: "0%",
                25: "",
                50: "25%",
                75: "",
                100: "100%",
                125: "",
                150: "150%",
                175: "",
                200: "200%",
                225: "",
                250: "250%",
                275: "",
                300: "300%",
            },
            value=100,
        )
    ]
)


def stand_usage_stats(config: Configuration):
    return f"Число использованных стоянок: {len(config.stand_counts())} из {len(config.stands)}"


def generate_overview(config: Configuration):
    total_cost = config.flights["total_cost"].sum()
    cost_per_passenger = total_cost / config.flights["flight_PAX"].sum()
    cost_per_flight = total_cost / len(config.flights)
    average_jet_bridge = config.flights["use_jet_bridge"].mean()
    overview_lines = [
        f"Суммарная стоимость решения: {total_cost}",
        f"Средняя стоимость обслуживания пассажира: {cost_per_passenger:.1f}",
        f"Средняя стоимость обслуживания рейса: {cost_per_flight:.0f}",
        stand_usage_stats(config),
        f"Доля рейсов испольщующих телетрап: {average_jet_bridge * 100:.1f}%",
    ]
    return [html.P(s, className="mb-0") for s in overview_lines]


RIGHT_COLUMN = html.Div([
    dcc.Loading(
        children=[
            dbc.CardHeader(html.H5("Общая статистика по решению")),
            dbc.CardBody(children=generate_overview(default_config), id="overview")
        ],
        type="default",
    )
])


def cost_chart(config: Configuration):
    costs = ['taxiing_cost', 'bus_cost', 'parking_cost']
    data = defaultdict(list)
    for ac_class in config.ac_classes():
        mask = config.flights["ac_class"] == ac_class
        for cost_type in costs:
            mean_cost = config.flights[mask][cost_type].mean()
            data["Aircraft class"].append(ac_class.replace('_', ' '))
            data["Cost type"].append(cost_type.replace('_', ' '))
            data["Average cost"].append(mean_cost)

    fig = px.bar(data, x="Aircraft class", y="Average cost",
                 color="Cost type", barmode="group")
    return fig


COST_CHART = html.Div([
    dcc.Loading(
        children=[
            dbc.CardHeader(html.H5("Распределение стоимостей по компонентам для разных типов самолетов")),
            dbc.CardBody([
                html.P(
                    "Стоимость обслуживания самолета складывается из:",
                    className="mb-0",
                ),
                html.Ul([
                    html.Li("Стоимости руления"),
                    html.Li("Стоимости перевозки пассажиров (в случае использования автобуса)"),
                    html.Li("Стоимости использования места стоянки"),
                ]),
                dcc.Graph(id="total-cost-chart", figure=cost_chart(default_config)),
            ])
        ],
        type="default",
    )
])


def flights_stats_chart(config: Configuration):
    features = ['handling_time', 'taxiing_time', 'use_jet_bridge']
    value_names = ['Average time', 'Average time', 'Ratio of using jet bridge']
    colors = px.colors.qualitative.Plotly

    ac_classes = config.ac_classes()
    fig = make_subplots(rows=1, cols=3,
                        subplot_titles=("Average handling time", "Average taxiing time", "Jet bridge usage"))
    for i, (feature, value_name) in enumerate(zip(features, value_names)):
        mean_values = []
        for ac_class in ac_classes:
            mask = config.flights["ac_class"] == ac_class
            mean_values.append(config.flights[mask][feature].mean())

        fig.add_trace(go.Bar(
            name=value_name,
            x=[ac_class.replace('_', ' ') for ac_class in ac_classes],
            y=mean_values,
            marker_color=colors[:len(ac_classes)]
        ), row=1, col=i + 1)

    fig.update_layout(showlegend=False)
    return fig


FLIGHT_STATS_CHART = html.Div([
    dcc.Loading(
        children=[
            dbc.CardHeader(html.H5("Статистика обслуживания для разных типов самолетов")),
            dbc.CardBody([
                html.P(
                    "Для разных типов самолетов отличаются:",
                    className="mb-0",
                ),
                html.Ul([
                    html.Li("Время руления"),
                    html.Li("Время обработки рейса"),
                    html.Li("Использование телетрапа"),
                ]),
                dcc.Graph(id="flight-stats-chart", figure=flights_stats_chart(default_config)),
            ])
        ],
        type="default",
    )
])


def convert_counter(counter: Counter):
    keys = list(counter.keys())
    values = [counter[k] for k in keys]
    return keys, values


def stands_chart(config: Configuration, use_time: bool):
    flights_mapping = config.flights_mapping
    stands_jet_bridge = Counter()
    stands_bus = Counter()
    for flight, stand_id in enumerate(flights_mapping):
        delta = 1 if not use_time else config.flights.loc[flight]['handling_time']
        if config.flights.loc[flight]['use_jet_bridge']:
            stands_jet_bridge[stand_id] += delta
        else:
            stands_bus[stand_id] += delta

    my_data = [
        go.Bar(x=convert_counter(stands_bus)[0], y=convert_counter(stands_bus)[1], orientation='v', name='Bus'),
        go.Bar(x=convert_counter(stands_jet_bridge)[0], y=convert_counter(stands_jet_bridge)[1], orientation='v',
               name='Jet bridge')
    ]
    if not use_time:
        my_layout = ({"title": "Число обслуженных рейсов для каждого места стоянки",
                      "yaxis": {"title": "Число рейсов"},
                      "xaxis": {"title": "Места стоянки"}})
    else:
        my_layout = ({"title": "Суммарное время обслуживания самолетов для каждого места стоянки",
                      "yaxis": {"title": "Время (в минутах)"},
                      "xaxis": {"title": "Места стоянки"}})

    fig = go.Figure(data=my_data, layout=my_layout)
    fig.update_layout(barmode='stack')
    return fig


STANDS_CHART = html.Div([
    dcc.Loading(
        children=[
            dbc.CardHeader(html.H5("Статистика загруженности мест стоянки")),
            dbc.CardBody([
                html.P(
                    id="stands-usage-stats",
                    children=[stand_usage_stats(default_config)],
                    className="mb-0",
                ),
                dcc.Graph(id="stand-counts-chart", figure=stands_chart(default_config, use_time=False)),
                dcc.Graph(id="stand-times-chart", figure=stands_chart(default_config, use_time=True))
            ])
        ],
        type="default",
    )
])

BODY = dbc.Container(
    [
        dbc.Row(
            [
                dbc.Col(LEFT_COLUMN, md=6, align="center"),
                dbc.Col(dbc.Card(RIGHT_COLUMN), md=6, align="center")
            ],
            style={"marginTop": 30},
        ),
        dbc.Card(COST_CHART, style={"marginTop": 30}),
        dbc.Card(FLIGHT_STATS_CHART, style={"marginTop": 30}),
        dbc.Card(STANDS_CHART, style={"marginTop": 30}),
    ],
    className="mt-12",
)

app = dash.Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP])
server = app.server  # for Heroku deployment

app.layout = html.Div(children=[NAVBAR, BODY])


@app.callback(
    [
        Output('total-cost-chart', 'figure'),
        Output('flight-stats-chart', 'figure'),
        Output('stands-usage-stats', 'children'),
        Output('stand-counts-chart', 'figure'),
        Output('stand-times-chart', 'figure'),
        Output('overview', 'children'),
    ],
    [Input("n-selection-slider", "value")],
)
def handle_percentage(percentage):
    generate_new_timetable(percentage, default_config)
    rerun_model(percentage, default_config)
    if percentage != 100:
        new_config = get_config(f"Solution_{percentage}.csv")
    else:
        new_config = default_config
    return cost_chart(new_config), \
           flights_stats_chart(new_config), \
           [stand_usage_stats(new_config)], \
           stands_chart(new_config, use_time=False), \
           stands_chart(new_config, use_time=True), \
           generate_overview(new_config)


if __name__ == "__main__":
    app.run_server(debug=True)
