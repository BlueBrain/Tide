import Debounder from '../debouncer'
import Listeners from '../listeners'


export interface IDimension {
    width: number,
    height: number
}

interface IElemSize {
    blockSize: number,
    inlineSize: number
}

interface IResizeObserverEntry {
    borderBoxSize: IElemSize,
    contentBoxSize: IElemSize,
    contentRect: DOMRectReadOnly,
    target: Element | SVGElement
}

interface ITarget {
    target: Element | SVGElement,
    rect: DOMRectReadOnly
}

const ResizeObserver = class {
    private intervalId = 0
    private targets: ITarget[] = []

    constructor(private slot: (entries: IResizeObserverEntry[]) => void) {}

    observe(target: Element | SVGElement) {
        const { targets } = this
        if (targets.map(getTarget).indexOf(target) !== -1) return
        if (targets.length === 0) {
            this.intervalId = window.setInterval(this.checkSize, 200)
        }
        targets.push({
            target,
            rect: target.getBoundingClientRect() as DOMRectReadOnly
        })
    }

    unobserve(target: Element | SVGElement) {
        const { targets } = this
        const indexOf = targets.map(getTarget).indexOf(target)
        if (indexOf === -1) return
        targets.splice(indexOf, 1)
        if (targets.length === 0) {
            window.clearInterval(this.intervalId)
        }
    }

    private checkSize = () => {
        const entries: IResizeObserverEntry[] = []

        for( const item of this.targets ) {
            const rect = item.target.getBoundingClientRect() as DOMRectReadOnly
            if (areDifferent(rect, item.rect)) {
                const size = {
                    inlineSize: rect.width,
                    blockSize: rect.height
                }
                entries.push({
                    target: item.target,
                    borderBoxSize: size,
                    contentBoxSize: size,
                    contentRect: rect
                })
                item.rect = rect
            }
        }

        if (entries.length > 0) {
            this.slot(entries)
        }
    }
}

export default class ResizeWatcher {
    private listeners = new Listeners<IDimension>()
    private readonly observer: ResizeObserver

    constructor(private element: HTMLElement, private debouncerDelay: number = 300) {
        this.observer = new ResizeObserver(this.onEntries)
        this.fire = Debounder(this.fire, debouncerDelay)
    }

    subscribe(listener: (dimension: IDimension) => void) {
        if (this.listeners.length === 0) {
            this.attach()
        }
        this.listeners.add(listener)
    }

    unsubscribe(listener: (dimension: IDimension) => void) {
        this.listeners.remove(listener)
        if (this.listeners.length === 0) {
            this.detach()
        }
    }

    private attach() {
        this.observer.observe(this.element)
    }

    private detach() {
        this.observer.unobserve(this.element)
    }

    private fire = (width: number, height: number) => {
        this.listeners.fire({ width, height })
    }

    private onEntries = (entries: IResizeObserverEntry[]) => {
        const filteredEntries = entries.filter((item: IResizeObserverEntry) => item.target === this.element)
        if (filteredEntries.length > 0) {
            const entry = filteredEntries[0]
            const size = entry.contentBoxSize
            this.fire(size.inlineSize, size.blockSize)
        }
    }
}


function getTarget(obj: ITarget) {
    return obj.target
}


function areDifferent(r1: DOMRectReadOnly, r2: DOMRectReadOnly): boolean {
    if (r1.width !== r2.width) return true
    if (r1.height !== r2.height) return true

    return false
}
