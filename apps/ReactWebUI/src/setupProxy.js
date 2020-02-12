const proxy = require('http-proxy-middleware');

const MAP = {
    '/tide/0': 'http://localhost:8890/tide',
    '/tide/3': 'http://bbpav03.bbp.epfl.ch:8888/tide',
    '/tide/4': 'http://cave1.bbp.epfl.ch:8888/tide',
    '/tide/5': 'http://bbpav05.bbp.epfl.ch:8888/tide',
    '/tide/6': 'http://bbpav06.bbp.epfl.ch:8888/tide'
}

function removePrefix(prefix, name) {
    return name.substr(prefix.length)
}

module.exports = function(app) {
    app.use('/kibana', proxy({
        target: 'https://bbpteam.epfl.ch/viz/tide.sauron/',
        changeOrigin: true,
        pathRewrite: removePrefix.bind(null, '/kibana')
    }))
    for (const from of Object.keys(MAP)) {
        const to = MAP[from]
        app.use(from, proxy({
            target: to,
            changeOrigin: true,
            pathRewrite: removePrefix.bind(null, '/tide/*')
        }))
    }
}
