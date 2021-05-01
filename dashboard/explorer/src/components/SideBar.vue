<template>
  <div class="text-left">
    <b-button v-b-toggle.sidebar-1>Nueva búsqueda</b-button>
    <b-sidebar id="sidebar-1" title="Parámetros de entrada" shadow>
        <h3> Sensor </h3>
        <select v-model="co2.device_id">
          <option v-for="(id, index) in device_ids" :key="index" class="dropdown-item" v-bind:value="id"> 
             {{ id }}
          </option>
        </select>
        <!-- <div class="dropdown">
            <button class="btn btn-secondary dropdown-toggle" type="button" id="dropdownMenuButton" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
            Sensor
            </button>
            <div class="dropdown-menu">
                <a v-for="(id, index) in device_ids" :key="index" class="dropdown-item" href="#"> 
                  {{ id }}
                </a>
            </div> 
        </div> -->
        <p> </p>
        <div id="event_period">
            <h3> Tiempo inicial </h3>
            <input type="datetime-local" class="actual_range" v-model="co2.min_isotime">
            <p> </p>
            <h3> Tiempo máximo </h3>
            <input type="datetime-local" class="actual_range" v-model="co2.max_isotime">
        </div>
        <p> </p>
        <button v-on:click="$emit('update')"> Actualizar </button>
        <p> </p>
        {{ co2.loading_status }}
        {{ co2.last_update }}
    </b-sidebar>
  </div>
</template>


<script>
export default({
  props: {
    co2: Object,
    device_ids: Array
  }
})
</script>
