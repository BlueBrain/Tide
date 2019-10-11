const proxy = require('http-proxy-middleware');

const MAP = {
    '/tide/00': 'http://localhost:8890/tide',
    '/tide/05': 'http://bbpav05.bbp.epfl.ch:8888/tide',
    '/tide/06': 'http://cave1.bbp.epfl.ch:8888/tide'
}

function removePrefix(name) {
    return name.substr('/tide/00'.length)
}

module.exports = function(app) {
    for (const from of Object.keys(MAP)) {
        const to = MAP[from]
        app.use(from, proxy({
            target: to,
            changeOrigin: true,
            pathRewrite: removePrefix
        }));
    }
}
