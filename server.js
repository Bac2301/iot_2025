const express = require('express');
const path = require('path');
const app = express();

app.use(express.urlencoded({ extended: true }));
app.use(express.json());

let sensorDataList = [];

app.use(express.static(path.join(__dirname)));

app.post('/data', (req, res) => {
  const { temperature, humidity, soilpercent, lightpercent } = req.body;

  if (!temperature && !humidity && !soilpercent && !lightpercent) {
    return res.status(400).json({ error: 'No sensor data provided' });
  }

  const sensorData = {
    temperature: temperature !== undefined && temperature !== null ? temperature : null,
    humidity: humidity !== undefined && humidity !== null ? humidity : null,
    soilpercent: soilpercent !== undefined && soilpercent !== null ? soilpercent : null,
    lightpercent: lightpercent !== undefined && lightpercent !== null ? lightpercent : null,
    timestamp: new Date().toISOString()
  };

  sensorDataList.push(sensorData); 
  console.log('Received sensor data:', sensorData);
  res.json({ sensorData }); 
});

app.get('/data', (req, res) => {
  res.json({ sensorDataList: sensorDataList });
});

app.listen(3000, () => {
  console.log('Server running on http://192.168.87.176:3000');   //http://192.168.87.176:3000
});