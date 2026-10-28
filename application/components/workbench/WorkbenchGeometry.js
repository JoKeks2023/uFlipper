.pragma library

var unit = 8
var gap = 2 * unit
var padding = 3 * unit
var controlHeight = 5 * unit
var detailWidth = 30 * unit

function snapUp(value) { return Math.ceil(value / unit) * unit }
function snapDown(value) { return Math.max(0, Math.floor(value / unit) * unit) }
