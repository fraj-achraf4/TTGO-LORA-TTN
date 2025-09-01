function Decoder(bytes, port) {
  var decoded = {};

  // Température: 2 octets, signé, diviser par 100
  decoded.temperature = ((bytes[0] << 8) | bytes[1]) / 100;

  // Humidité: 2 octets, non signé, diviser par 100
  decoded.humidity = ((bytes[2] << 8) | bytes[3]) / 100;

  // Pression: 4 octets, non signé, diviser par 100 pour obtenir hPa
  decoded.pressur = ((bytes[4] << 24) | (bytes[5] << 16) | (bytes[6] << 8) | bytes[7]) / 100;
  
  // Gaz: 3 octets, non signé, diviser par 1000 pour obtenir KOhms
  decoded.gaz = ((bytes[8] << 16) | (bytes[9] << 8) | bytes[10]) / 1000;

  return decoded;
}