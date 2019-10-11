const Path = require('path')
const exec = require("child_process").exec

const cwd = process.cwd()
const pos = cwd.indexOf('/Tide/')
const root = Path.resolve(cwd.substr(0, pos))
const tidePath = Path.join(root, 'Tide/Build/bin/tide')
const configPath = Path.join(root, 'Tide/apps/ReactWebUI/backend')

console.info("tidePath=", tidePath)
console.info("configPath=", configPath)
console.log("")

console.log("Starting fake Tide servers...")
console.log("+-------+------+")
console.log("| Level | Port |")
console.log("+-------+------+")
const levels = [0/*, 5, 6*/]
levels.forEach( level => {
    console.log(`|   ${level}   | 889${level} |`)
    const cmd = `"${tidePath}" --config "${configPath}/wall-${level}.json"`
    exec(cmd, (err, stdout, stderr) => {
        if (err) {
            console.error('')
            console.error(`### Failure on level ${level}!`)
            console.error('>>>', cmd)
            console.error(err)
        }
    })
})
console.log("+-------+------+")
