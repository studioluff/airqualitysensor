//___________ AQI CONVERSION___________ 
/*
 * Values derived from Airnow.gov
 * https://www.airnow.gov/sites/default/files/2020-05/aqi-technical-assistance-document-sept2018.pdf
 * 
 */
float AQIvalue;

int AQIVal(int ppmVal){
  
  ppmVal *= 10;
  
  if(ppmVal <= 120.0){
    //0-50 AQI | Green | Good
    AQIvalue = map(ppmVal, 0.0, 120.0, 0.0, 500.0);
    }
  else if (ppmVal > 120.0 && ppmVal <=354.0){
    //51-100 AQI | Yellow | Moderate
    AQIvalue = map(ppmVal, 120.0, 354.0, 510.0, 1000.0);
    }
  else if (ppmVal > 354.0 && ppmVal <=554.0){
    //101-150 AQI | Orange | Unhealthy for sensitive groups
    AQIvalue = map(ppmVal, 354.0, 554.0, 1010.0, 1500.0);
    }
  else if (ppmVal > 554.0 && ppmVal <=1504.0){
    //151-200 AQI | Red | Unhealthy
    AQIvalue = map(ppmVal, 554.0, 1504.0, 1510.0, 2000.0);
    }
  else if (ppmVal > 1504.0 && ppmVal <=2504.0){
    //201-300 AQI | Purple | Very Unhealthy
    AQIvalue = map(ppmVal, 1504.0, 2504.0, 2010.0, 3000.0);
    }
  else if (ppmVal > 2504.0 && ppmVal <=3504.0){
    //301-400 AQI | Marone | Hazardous
    AQIvalue = map(ppmVal, 2504.0, 3504.0, 3010.0, 4000.0);
    }
  else if (ppmVal > 3504.0 && ppmVal <=5000.0){
    //401-500 AQI | Marone | Hazardous
    AQIvalue = map(ppmVal, 3504.0, 5000.0, 4010.0, 5000.0);
    }
  
  AQIvalue /= 10.0;
  return round(AQIvalue);
  }
