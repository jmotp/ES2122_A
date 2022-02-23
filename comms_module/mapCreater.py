import serial
from gmplot import gmplot

apikey = 'AIzaSyAlvIiBa_MLVlVa5Non1DqIikJuow4c0qQ'  # (your API key here)
feup_coords = [41.17787232359886, -8.59588657762354]
gmap = gmplot.GoogleMapPlotter(*feup_coords, 19, apikey=apikey)
coords = []
offsetlatt = 41.17816248879532 - 41.17790380424444
offsetlong = -8.594879393782438 + 8.595108254882549

def addToMap(lattitude, longitude):
    # Create the map plotter:
    coords.append([lattitude, longitude])
    positions = zip(*coords)
    gmap.plot(*positions, color='purple', edge_width=5)
    gmap.circle(lattitude, longitude, 2, color='red')
    gmap.draw('map.html')
    # addAutoRefresh()


def addAutoRefresh():
    mapFile = open("map.html", "a")
    mapFile.seek(0, 2)
    mapFile.write("<meta http-equiv=\"refresh\" content=5>\n")


addToMap(41.17799022289902, -8.594894628019045)
addToMap(41.1779459242308, -8.595416970678802)
addToMap(41.17808989479296, -8.595593537211677)
