<template>
  <div id="app" class="container">
    <SideBar :co2="co2" :device_ids="device_ids" v-on:update="update" />
    <TimeSeries :co2="co2" :dataSource="dataSource" />
  </div>
</template>

<script>
import SideBar from "./components/SideBar.vue";
import TimeSeries from "./components/TimeSeries.vue";
import FusionCharts from "fusioncharts";

var jsonify = (res) => res.json();
const co2api = "https://1jakqf24xb.execute-api.us-east-2.amazonaws.com/dev/co2";
var schema = [
  {
    name: "Time",
    type: "date",
    format: "%Y-%-m-%-dT%H:%M:%S",
  },
  {
    name: "CO2 [ppm]",
    type: "number",
  },
  {
    name: "Temperature [C]",
    type: "number",
  },
];
console.log(schema);

export default {
  name: "App",
  components: {
    SideBar,
    TimeSeries,
  },
  data() {
    return {
      co2: {
        data: [],
        datafc: [],
        device_id: 1,
        min_isotime: "2021-03-29T00:00:00",
        max_isotime: "2021-03-29T20:00:00",
        last_update: "",
        loading_status: "Cargando datos..."
      },
      device_ids: [0, 1, 2],
      dataSource: {
        data: null,
        chart: {
          multicanvas: true,
        },
        caption: {
          text: null,
        },
        subcaption: {
          text: "Variables ambientales",
        },
        yAxis: [
          {
            plot: {
              value: "CO2 [ppm]",
              connectnulldata: true,
              type: "line",
            },
            title: "CO2",
            format: {
              suffix: " ppm",
            },
            style: {
              title: {
                "font-size": "20px",
              },
            },
            referenceLine: [
              {
                label: "Ventilar",
                value: "500",
              },
            ],
          },
          {
            plot: {
              value: "Temperature [C]",
              connectnulldata: true,
              type: "line",
            },
            title: "Temperatura",
            format: {
              suffix: " C",
            },
            style: {
              title: {
                "font-size": "20px",
              },
            },
          },
        ],
      },
    };
  },
  methods: {
    filldata(res) {
      console.log("filldata...")
      var i;
      this.co2.last_update = res.now;
      for (i = 0; i < res.data.length; i++) {
        this.co2.datafc.push([
          res.data[i].isotime.substring(0, 19), //remove decimal part in seconds
          parseFloat(res.data[i].co2),
          parseFloat(res.data[i].temperature),
        ]);
      }
      return res.next_url
    },
    getAPIdata(url) {
      return fetch(url)
        .then(jsonify)
        .then(this.filldata)
        .then((url) => {
          console.log(url)
          if (url != undefined) {
            return this.getAPIdata(url);
          } else {
            this.co2.loading_status = "Última actualización:"
          }
        });
    },
    parseAPIdata() {
      var url = `${co2api}?device_id=${this.co2.device_id}&min_isotime=${this.co2.min_isotime}&max_isotime=${this.co2.max_isotime}`;
      this.co2.datafc = [];
      this.co2.loading_status = "Cargando datos..."
      this.getAPIdata(url).then(() => {
        console.log("Parsing API response data...");

        this.dataSource.caption.text = `Sensor: ${this.co2.device_id}`;
        const fusionTable = new FusionCharts.DataStore().createDataTable(
          this.co2.datafc,
          schema
        );
        this.dataSource.data = fusionTable;
        console.log("dataSource");
        console.log(this.dataSource);
      });
    },
    update() {
      this.parseAPIdata();
    },
  },
  mounted: function () {
    this.parseAPIdata();
  },
};
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}
</style>
