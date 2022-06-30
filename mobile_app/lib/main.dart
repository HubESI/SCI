import 'dart:async';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:google_maps_flutter/google_maps_flutter.dart';
import 'package:location/location.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

void main() => runApp(const MyApp());

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  static double latitude = 2.0, longitude = 34.0;
  static String timestamp = "";
  late GoogleMapController mapController;
  late LatLng source = LatLng(latitude, longitude);
  Marker markerObj = Marker(
    markerId: MarkerId("source"),
    position: LatLng(latitude, longitude),
    infoWindow: InfoWindow(
      title: 'Tracked object',
      snippet: 'My object is here',
    ),
  );

  Future<int> connect() async {
    final client = MqttServerClient("broker.mqttdashboard.com", 'SCI');
    client.setProtocolV311();
    client.logging(on: false);
    client.keepAlivePeriod = 20;
    client.onDisconnected = onDisconnected;
    client.onSubscribed = onSubscribed;
    final connMess = MqttConnectMessage()
        .withClientIdentifier('Mqtt_MyClientUniqueIdQ1')
        .withWillTopic('willtopic') // If you set this you must set a will message
        .withWillMessage('My Will message')
        .startClean() // Non persistent session for testing
        .withWillQos(MqttQos.atLeastOnce);
    print('::Mosquitto client connecting....');
    client.connectionMessage = connMess;

    try {
      await client.connect();
    } on Exception catch (e) {
      print('::client exception - $e');
      client.disconnect();
    }

    if (client.connectionStatus!.state == MqttConnectionState.connected) {
      print('::Mosquitto client connected');
    } else {
      print('::ERROR Mosquitto client connection failed - disconnecting, state is ${client.connectionStatus!.state}');
      client.disconnect();
      exit(-1);
    }

    print('<<<< SUBSCRIBE >>>>');
    const topic1 = 'geoLocation';
    client.subscribe(topic1, MqttQos.exactlyOnce);
    print(' + SUBSCRIBED');

    client.updates!.listen((List<MqttReceivedMessage<MqttMessage?>>? c) {
      final recMess = c![0].payload as MqttPublishMessage;
      final pt = MqttPublishPayload.bytesToStringAsString(recMess.payload.message!);
      print("pt ==> $pt");
      List<String> s = pt.split(" ");

      longitude = double.parse(s[0].split("=")[1]) ;
      latitude  = double.parse(s[1].split("=")[1]);
      timestamp = s[2].split("=")[1];
      print("******* latitude $latitude");
      print("******* longitude $longitude");
      print("timestamp $timestamp");

      source = LatLng(longitude, latitude);
      _setMarker();
      print('EXAMPLE::Change notification:: topic is <${c[0].topic}>, payload is <-- $longitude, $latitude -->');
      print('');
    });

    client.published!.listen((MqttPublishMessage message) {
      print('::Published notification:: topic is ${message.variableHeader!.topicName}, with Qos ${message.header!.qos}');
    });

    //print('::Sleeping....');
    //await MqttUtilities.asyncSleep(60);

    //print('::Unsubscribing');
    //client.unsubscribe(topic1);

    //await MqttUtilities.asyncSleep(1000);
    //print('::Disconnecting');
    //client.disconnect();
    return 0;
  }

  void onSubscribed(String topic) {
    print('::Subscription confirmed for topic $topic');
  }

  void onDisconnected() {
    print('::OnDisconnected client callback - Client disconnection');
  }

  void _onMapCreated(GoogleMapController controller) {
    mapController = controller;
  }

  @override
  void initState() {
    super.initState();
    connect();
  }

  void _setMarker() {
    setState(() {
      markerObj = Marker(
        markerId: MarkerId("source"),
        position: LatLng(longitude, latitude),
        infoWindow: InfoWindow(
          title: 'Tracked object',
          snippet: timestamp,
        ),
      );
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Projet SCI'),
          backgroundColor: Colors.green[700],
        ),
        body: GoogleMap(
          onMapCreated: _onMapCreated,
          initialCameraPosition: CameraPosition(
            target: source,
            zoom: 11.0,
          ),
          markers: {
            markerObj,
          },
        ),
      ),
    );
  }
}