function decodeUplink(input) {
  const bytes = input.bytes;

  const gas = bytesToFloat(bytes.slice(0, 4));
  const humidity = bytesToFloat(bytes.slice(4, 8));
  const pressure = bytesToFloat(bytes.slice(8, 12));
  const temperature = bytesToFloat(bytes.slice(12, 16));

  return {
    data: {
      gas: parseFloat(gas.toFixed(2)),
      humidity: parseFloat(humidity.toFixed(2)),
      pressure: parseFloat(pressure.toFixed(0)),
      temperature: parseFloat(temperature.toFixed(2))
    },
    warnings: [],
    errors: []
  };
}

function bytesToFloat(bytes) {
  const buffer = new ArrayBuffer(4);
  const view = new DataView(buffer);
  bytes.forEach((b, i) => view.setUint8(i, b));
  return view.getFloat32(0, true); // little-endian
}