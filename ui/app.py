from collections import defaultdict

import dash
import dash_bootstrap_components as dbc
import dash_core_components as dcc
import dash_html_components as html
import plotly.express as px
import plotly.graph_objects as go
from plotly.subplots import make_subplots

from prepare_plots import get_config

config = get_config()

PLOTLY_LOGO = "https://images.plot.ly/logo/new-branding/plotly-logomark.png"

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
            min=1,
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


def cost_chart():
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
        dcc.Graph(id="total-cost-chart", figure=cost_chart()),
    ])
])


def flights_stats_chart():
    features = ['handling_time', 'taxiing_time', 'use_jet_bridge']
    value_names = ['Average time', 'Average time', 'Ratio of using jet bridge']
    colors = px.colors.qualitative.Plotly

    ac_classes = config.ac_classes()
    fig = make_subplots(rows=1, cols=3, subplot_titles=("Average handling time", "Average taxiing time", "Jet bridge usage"))
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
        dcc.Graph(id="flight-stats-chart", figure=flights_stats_chart()),
    ])
])

STANDS_CHART = html.Div([
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
        dcc.Graph(id="flight-stats-chart", figure=flights_stats_chart()),
    ])
])

BODY = dbc.Container(
    [
        dbc.Row(
            [
                dbc.Col(LEFT_COLUMN, md=6, align="center")
                # dbc.Col(dbc.Card(TOP_BANKS_PLOT), md=8),
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

"""
#  Callbacks
"""


if __name__ == "__main__":
    app.run_server(debug=True)
