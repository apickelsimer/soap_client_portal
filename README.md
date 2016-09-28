# soap_client_portal
This is a custom drupal module that allows for management and client testing of SOAP services.

Dependancies:

wsclient (https://www.drupal.org/project/wsclient)

Getting Started:

1. Install and enable the dependancies listed above.

2. Install and enable soap_client

3. Upload a WSDL into wsclient (admin/config/services/wsclient). If using a Drupal hosted service such as Pantheon, you may have to upload the WSDL as a file instead of referencing an external URL. A sample has been provided in this repo (see weather.txt).

4. Once the WDL is uploaded, you can navigate to newly created service page to see available operations at 'soap/%wsclientservice' (e.g. soap/weather)

5. Each operation will have a unique endpoint provided as a web based client at 'soap/%wsclientservice/operation/%operationid/invoke' (e.g. soap/weather/GetCityWeatherByZIP/invoke)

