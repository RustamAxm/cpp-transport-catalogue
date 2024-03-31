from transport_catalogue_pb2 import TransportCatalogue


def main():
    base = TransportCatalogue()

    with open('../transport_catalogue.db', 'rb') as file:
        base.ParseFromString(file.read())

    for stop in base.transport_catalogue.stops:
        print(stop)


if __name__ == '__main__':
    main()
