<template>
  <div id="app" class="container">
    <SideBar 
      :co2="co2" 
      :device_id_prefixes="device_id_prefixes" 
      :device_id_suffixes="device_id_suffixes"
      :device_id_longnames="device_id_longnames"
      :device_id_codes="device_id_codes"
      :device_id_maxsensor="device_id_maxsensor"
      v-on:update="update"
      v-on:reset_mintime="reset_mintime"
      v-on:reset_maxtime="reset_maxtime" 
      v-on:check_times="check_times"
    />
    <TimeSeries 
      :co2="co2" 
      :dataSource="dataSource"
      :device_id_longnames="device_id_longnames"
      :device_id_codes="device_id_codes"
      :device_id_maxsensor="device_id_maxsensor"
      :device_id_co2stats="device_id_co2stats"
      :device_id_tempstats="device_id_tempstats"
    />
  </div>
</template>

<script>
import Vue from 'vue';
import SideBar from "./components/SideBar.vue";
import TimeSeries from "./components/TimeSeries.vue";
import FusionCharts from "fusioncharts";

var jsonify = (res) => res.json();
const co2api = "https://8du9q3bcpj.execute-api.us-east-2.amazonaws.com/dev/co2";
//const co2api = "https://1jakqf24xb.execute-api.us-east-2.amazonaws.com/dev/co2";
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
  {
    name: "Humidity [%]",
    type: "number",
  }
];
console.log(schema);

export default {
  name: "App",
  components: {
    SideBar,
    TimeSeries
  },
  data() {
    return {
      co2: {
        data: [],
        datafc: [],
        device_id_prefix: "Oficina Francisco", // only for the first image
        device_id_suffix: 1, // note that suffix starts with zero, but sensors start with one
        min_isotime: "", //2021-05-03T02:30:00",
        max_isotime: "", //2021-05-03T03:00:00",
        last_update: "",
        loading_status: "Cargando datos...",
        query: 1      },
      device_id_longnames: [],
      device_id_comunas: {'Oficina Francisco': 'Providencia'},  // first values, just to show when first entering the page
      device_id_codes: {'Oficina Francisco': 'ffb'}, // first values, just to show when first entering the page
      device_id_co2stats: {},
      device_id_tempstats: {},
      device_id_maxsensor: {},
      device_id_prefixes: [],
      device_id_suffixes: [],
      dataSource: {
        data: null,
        chart: {
          multicanvas: true,
        },
        navigator: {
          enabled: true,
          label: 'Navegar'
        },
        extensions: {
          customRangeSelector: {
            enabled: "0"
          }
        },
        legend: {
          enabled: "0"
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
                value: "800",
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
          {
            plot: {
              value: "Humidity [%]",
              connectnulldata: true,
              type: "line",
            },
            title: "Humedad",
            format: {
              suffix: " %",
            },
            style: {
              title: {
                "font-size": "20px",
              },
            },
          }
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
          parseFloat(res.data[i].humidity),
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
            this.co2.loading_status = "Última actualización (UT):"
          }
        });
    },
    parseAPIdata() {
      var sprintf = require('sprintf-js').sprintf;
      var url, i, device_id_suffix_formatted;
      if (this.co2.device_id_suffix != "Todos") {
        device_id_suffix_formatted = sprintf('%02d', this.co2.device_id_suffix);
        if ((this.co2.min_isotime != "") & (this.co2.max_isotime != "")) {
          url = `${co2api}?device_id=${this.device_id_codes[this.co2.device_id_prefix]}-${device_id_suffix_formatted}&min_isotime=${this.co2.min_isotime}&max_isotime=${this.co2.max_isotime}`;
        } else if ((this.co2.min_isotime === "") & (this.co2.max_isotime === "")) {
          url = `${co2api}?device_id=${this.device_id_codes[this.co2.device_id_prefix]}-${device_id_suffix_formatted}`;
        }
        console.log(url)
        this.co2.datafc = [];
        this.co2.loading_status = "Cargando datos...";
        this.getAPIdata(url).then(() => {
          console.log("Parsing API response data...");

          this.dataSource.caption.text = `Sensor: ${this.co2.device_id_prefix}, ${this.device_id_comunas[this.co2.device_id_prefix]} (${this.device_id_codes[this.co2.device_id_prefix]}-${device_id_suffix_formatted})`;
          const fusionTable = new FusionCharts.DataStore().createDataTable(
            this.co2.datafc,
            schema
          );
          this.dataSource.data = fusionTable;
        });
      } else {
        for (i=0; i < this.device_id_maxsensor[this.co2.device_id_prefix]; i++) {
          device_id_suffix_formatted = sprintf('%02d', i + 1);
          url = `${co2api}?device_id=${this.device_id_codes[this.co2.device_id_prefix]}-${device_id_suffix_formatted}`;
          console.log(url);
          this.get_stats(url, i);
        }
      }
    },
    update() {
      this.parseAPIdata();
    },
    reset_mintime() {
      this.co2.min_isotime = "";
      if (this.co2.max_isotime != "") {
        this.co2.query = 0;
      } else {
        this.co2.query = 1;
      }
    },
    reset_maxtime() {
      this.co2.max_isotime = "";
      if (this.co2.min_isotime != "") {
        this.co2.query = 0;
      } else {
        this.co2.query = 1;
      }
    },
    check_times() {
      console.log(this.co2.min_isotime);
      if (((this.co2.min_isotime != "") & (this.co2.max_isotime != "")) | ((this.co2.min_isotime === "") & (this.co2.max_isotime === ""))) {
        // add here maximum time difference check
        this.co2.query = 1;
      } else {
        this.co2.query = 0;
      }
    },
    save_location(results) {
      var i, j;
        for (i = 1; i < results.data.length; i++) { 
          this.device_id_longnames.push(results.data[i][0]);
          this.device_id_comunas[results.data[i][0]] = results.data[i][1];
          this.device_id_codes[results.data[i][0]] = results.data[i][2];
          this.device_id_maxsensor[results.data[i][0]] = parseInt(results.data[i][3]);
          this.device_id_co2stats[results.data[i][0]] = []
          this.device_id_tempstats[results.data[i][0]] = []
          for (j = 0; j < this.device_id_maxsensor[results.data[i][0]]; j++) {
            this.device_id_co2stats[results.data[i][0]].push("");
            this.device_id_tempstats[results.data[i][0]].push("");
          }
        }
    },
    parse_locations(callBack) {
      var url = "https://docs.google.com/spreadsheets/d/e/2PACX-1vSwU19n2rNiiZT_j4eX-IDKbj0jsFZ2NDly8PfWuUszTsH8unKtP4gqS9l-tys68578S0JIH3qBcpuu/pub?output=csv";
      this.$papa.parse(url, {
        download: true, complete: function(results) { 
          callBack(results);
        }
      });
    },
    parse_stats(res) {
      var id = res.data[0]["device_id"];
      id = parseInt(id.substring(4, 6));
      var co2stats = [];
      var tempstats = [];
      var i;
      for (i = 0; i < res.data.length; i++) {
        co2stats.push(parseFloat(res.data[i].co2));
        tempstats.push(parseFloat(res.data[i].temperature));
      }
      Vue.set(this.device_id_co2stats[this.co2.device_id_prefix], id - 1, Math.max(...co2stats));
      Vue.set(this.device_id_tempstats[this.co2.device_id_prefix], id - 1, Math.min(...tempstats));
      this.co2.last_update = res.now;
      this.co2.loading_status = "Última actualización (UT)";
    },
    get_stats(url) {
      this.co2.loading_status = "Cargando datos...";
      fetch(url).then(jsonify).then(this.parse_stats);
    }
  },
  mounted: function () {
    this.parse_locations(this.save_location);
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
