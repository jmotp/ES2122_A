import serial
from gmplot import gmplot

serialPort = serial.Serial(
    port="COM6",
    baudrate=115200,
    bytesize=8,
    timeout=2,
    stopbits=serial.STOPBITS_ONE
)

apikey = 'INSERT_API_KEY'  # (your API key here)
feup_coords = [41.17787232359886, -8.59588657762354]
gmap = gmplot.GoogleMapPlotter(*feup_coords, 19, apikey=apikey)
coords = []

def addToMap(lattitude, longitude):
    # Create the map plotter:
    coords.append([lattitude, longitude])
    positions = zip(*coords)
    gmap.plot(*positions, color='purple', edge_width=5)
    gmap.circle(lattitude, longitude, 2, color='red')
    gmap.draw('map.html')
    addAutoRefresh()


def addAutoRefresh():
    mapFile = open("map.html", "a")
    mapFile.seek(0, 2)
    mapFile.write("<meta http-equiv=\"refresh\" content=5>\n")

#addToMap(41.1779459242308, -8.595416970678802)
#addToMap(41.17808989479296, -8.595593537211677)
serialString = ""

while 1:
    # Wait until there is data waiting in the serial buffer
    if serialPort.in_waiting > 0:

        serialString = serialPort.readline()
        msg = serialString.decode('Ascii')

        if msg.split(" ")[0] == "Packet":

            print(msg)
            if msg.split(";")[1].split(":")[0] == "Lat":
                lat = float(msg.split(";")[1].split(":")[1])

            if msg.split(";")[2].split(":")[0] == "Lng":
                lng = float(msg.split(";")[2].split(":")[1])
                addToMap(lat, lng)



