<template>
  <div class="text-left">
    <b-button v-b-toggle.sidebar-1>Nueva búsqueda</b-button>
    <b-sidebar id="sidebar-1" title="Parámetros de entrada" shadow>
        <h4> Colegio </h4>
        <div>
          <b-form-select v-model="co2.device_id_prefix" :options="device_id_longnames"></b-form-select>
        </div>
        <p> </p>
        <h5> Número de sensor </h5>
        <div>
          <b-form-select v-model="co2.device_id_suffix">                   
            <option v-for="(selectOption, indexOpt) in Array(device_id_maxsensor[co2.device_id_prefix]).keys()" 
            :key="indexOpt"
            :value="selectOption"
            >
              {{ selectOption + 1 }} 
            </option>
          </b-form-select>

          <!-- <b-form-select v-model="co2.device_id_suffix" :options="device_id_suffixes"></b-form-select> -->
        </div>
        <p> </p>
        <div class="accordion" id="accordionExample">
          <div class="card-header" id="headingOne">
            <h2 class="mb-0">
              <button class="btn btn-link" type="button" data-toggle="collapse" data-target="#collapseOne" aria-expanded="true" aria-controls="collapseOne">
                Seleccionar ventana de tiempo
              </button>
            </h2>
          </div>
            <div id="collapseOne" class="collapse" aria-labelledby="headingOne" data-parent="#accordionExample">
              <div class="card-body">
                <div id="event_period">
                  <h5> Tiempo inicial </h5>
                  <input class="form-control" type="datetime-local" v-model="co2.min_isotime" v-on:change="$emit('check_times')">
                  <p> </p>
                  <button class="btn btn-outline-dark btn-sm" v-on:click="$emit('reset_mintime')">Borrar</button>
                  <p> </p>
                  <h5> Tiempo final </h5>
                  <input class="form-control" type="datetime-local" v-model="co2.max_isotime" v-on:change="$emit('check_times')">
                  <p> </p>
                  <button class="btn btn-outline-dark btn-sm" v-on:click="$emit('reset_maxtime')">Borrar</button>
                </div>
              </div>
            </div>
          </div>
        <br>
        <button class="btn btn-primary" type="submit" v-on:click="$emit('update')" v-bind:disabled="co2.query === 0" > Actualizar </button>
        <p> </p>
        <div v-show="co2.loading_status === 'Cargando datos...'" class="spinner-border" role="status">
          <span class="sr-only">Loading...</span>
        </div>
        {{ co2.loading_status }}
        {{ co2.last_update }}
        
    </b-sidebar>
  </div>
</template>


<script>
export default({
  props: {
    co2: Object,
    device_id_prefixes: Array,
    device_id_suffixes: Array,
    device_id_longnames: Array,
    device_id_codes: Object,
    device_id_maxsensor: Object
  }
})
</script>
