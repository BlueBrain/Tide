import castInteger from '../../tfw/converter/integer'

export default {
    getWallsStatus
}

interface ISource {
    name: string,
    server: string,
    power: number,
    locked: number,
    status: number,
    surfaceSize: [number, number],
    proxy_endpoint: string,
    last_event_date: string
}

export interface IWallInfo {
    id: number,
    name: string,
    locked: boolean,
    power: boolean,
    powerIsUndef: boolean,
    width: number,
    height: number,
    lastInteraction: Date
}


async function getWallsStatus(): Promise<IWallInfo[]> {
    const now = new Date()
    const date = new Date(now.getFullYear(), now.getMonth(), now.getDate() - 1)
    const q = `{"range":{"time":{"gte":"${getKibanaFormattedDate(date)}","lte":"now"}}}`
    const sq = `"${q}"`

    // "kibana" is redirected by the proxy to the search service.
    const url = `kibana/_search?_source=last_event_date,name,deflect_port,locked,power,server,status,time,surfaceSize&q=${sq}&size=1200`
    const response = await fetch(url)
    const text = await response.text()
    const json = JSON.parse(text)
    if (json.error) {
        console.log(json.error.caused_by.reason)
        return []
    }
    const sources = json.hits.hits.map((hit: any) => hit._source)
        .sort(sortByDecreasingTime)
    const wallNames: Set<string> = new Set()
    const walls: IWallInfo[] = []
    sources.forEach((source: ISource) => {
        const { name, proxy_endpoint, locked, power, surfaceSize } = source
        if (typeof name !== 'string') return
        if (wallNames.has(name)) return
        wallNames.add(name)
        walls.push({
            name,
            id: castInteger(proxy_endpoint || `${name.charAt(4)}`, -1),
            width: surfaceSize[0],
            height: surfaceSize[1],
            locked: locked !== 0,
            power: power === 1,
            powerIsUndef: power === 2,
            lastInteraction: new Date(`${source.last_event_date}Z`)
        })
    })

    return walls.sort(sortByName)
}

/**
 * Kibana search has a very specific date formatting.
 */
function getKibanaFormattedDate(d: Date): string {
    // It looks like an ISO format.
    const iso = d.toISOString()
    // But without seconds and without trailing "Z".
    const kibanaDate = iso.substr(0, iso.length - ":00:00.000Z".length)
    // And a wierd escaped colon.
    return kibanaDate + "\\:00"
}


function sortByName(w1: IWallInfo, w2: IWallInfo) {
    const n1 = w1.name
    const n2 = w2.name
    if (n1 < n2) return -1
    if (n1 > n2) return +1
    return 0
}


function sortByDecreasingTime(s1: {time: string}, s2: {time: string}) {
    const d1 = s1.time
    const d2 = s2.time
    if (d1 > d2) return -1
    if (d1 < d2) return +1
    return 0
}
