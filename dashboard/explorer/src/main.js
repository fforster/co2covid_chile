import Vue from 'vue'
import BootStrapVue from 'bootstrap-vue'

import 'bootstrap/dist/js/bootstrap.bundle'
import 'bootstrap/dist/css/bootstrap.css'
import 'bootstrap-vue/dist/bootstrap-vue.css'

import VueFusionCharts from 'vue-fusioncharts';
import FusionCharts from 'fusioncharts';
import TimeSeries from 'fusioncharts/fusioncharts.timeseries';
import VuePapaParse from 'vue-papa-parse';

import App from './App.vue'

Vue.use(BootStrapVue);
Vue.use(VueFusionCharts, FusionCharts, TimeSeries);
Vue.use(VuePapaParse);
Vue.config.productionTip = false;

new Vue({
  render: h => h(App),
}).$mount('#app')
