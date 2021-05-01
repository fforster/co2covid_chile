<template>
    <div id="app">
        <fusioncharts
        :type="type"
        :width="width"
        :height="height"
        :dataSource="dataSource"
    ></fusioncharts>
    {{ co2.loading_status }}
    {{ co2.last_update }}
    </div>
</template>

<script>
export default({
    props: {
        co2: Object,
        dataSource: Object
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