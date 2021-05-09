<template>
    <div id="app">
        <div v-show="co2.loading_status === 'Cargando datos...'" class="spinner-border" role="status">
            <span class="sr-only">Loading...</span> 
        </div>
        <div v-if="this.co2.device_id_suffix != 'Todos'">
            {{ co2.loading_status }}
            {{ co2.last_update }}
            <fusioncharts 
            :type="type"
            :width="width"
            :height="height"
            :dataSource="dataSource">
            </fusioncharts>
        </div>
        <div v-else class="bv-example-row">
            <b-container>
                <h1> {{ co2.device_id_prefix}} ({{ device_id_codes[co2.device_id_prefix] }}) </h1>
                <p> Estadísticas de la última hora. </p>
                <p> {{ co2.loading_status }} {{ co2.last_update }} </p>
                <b-row cols="1" cols-sm="2" cols-md="4" cols-lg="6">
                    <b-col v-for="(item, index) in Array(device_id_maxsensor[co2.device_id_prefix]).keys()" :key="index">
                        <h4 style="color:black"> Sensor {{ item + 1}}: </h4>
                        <p> </p>
                        <p v-if="device_id_co2stats[co2.device_id_prefix][index] > 700">
                            Max. CO2: <span style="color:red"> {{ device_id_co2stats[co2.device_id_prefix][index] }} ppm </span>
                        </p>
                        <p v-else>
                            Max. CO2: <span style="color:green"> {{ device_id_co2stats[co2.device_id_prefix][index] }} ppm </span>
                        </p>
                        <p v-if="device_id_tempstats[co2.device_id_prefix][index] < 18">
                            Min. Temp: <span style="color:red"> {{ device_id_tempstats[co2.device_id_prefix][index] }} C </span>
                        </p>
                        <p v-else>
                            Min. Temp: <span style="color:green"> {{ device_id_tempstats[co2.device_id_prefix][index] }} C </span>
                        </p>
                        <br>
                        <br>
                    </b-col>
                </b-row>
            </b-container>
        </div>
        <hr>
        <a href="http://ingenieria.uchile.cl/"><img src="../assets/fcfm.jpg" height="120"></a> 
        <a href="http://www.cmm.uchile.cl/"><img src="../assets/cmm.png" height="70"></a>
        <a href="http://www.cata.cl/"><img src="../assets/cata.png" height="70"></a>
    </div>
    
</template>

<script>
export default({
    props: {
        co2: Object,
        dataSource: Object,
        dataSource_gauge: Object,
        device_id_longnames: Array,
        device_id_codes: Object,
        device_id_maxsensor: Object,
        device_id_tempstats: Object,
        device_id_co2stats: Object
    },
    name: 'app',
    data() {
        return {
            "type": "timeseries",
            "width": "100%",
            "height": "700"
        }
    }
})
</script>

<style scoped>
/* The container <div> - needed to position the dropdown content */
.dropdown {
  position: relative;
  display: inline-block;
}
</style>