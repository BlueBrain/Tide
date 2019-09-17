import * as React from "react"
import "../theme.css"
import "./input-date.css"
import castInteger from "../converter/integer"
import castBoolean from "../converter/boolean"
import castString from "../converter/string"
import Debouncer from "../debouncer"

import Intl from "../intl"
const _ = Intl.make(require("./input-date.yaml"));

interface INumberSlot {
    (value: number): void;
}

interface IInputProps {
    label?: string;
    value?: number;
    wide?: boolean;
    focus?: boolean;
    onChange?: INumberSlot
}

interface IInputState {
    error?: string;
}

export default class Input extends React.Component<IInputProps, IInputState> {
    readonly inputDay: React.RefObject<HTMLInputElement>;
    readonly inputMonth: React.RefObject<HTMLSelectElement>;
    readonly inputYear: React.RefObject<HTMLInputElement>;

    constructor(props: IInputProps) {
        super(props);
        this.state = {};
        this.handleChange = Debouncer(this.handleChange.bind(this), 300);
        this.inputDay = React.createRef();
        this.inputMonth = React.createRef();
        this.inputYear = React.createRef();
    }

    handleChange(): void {
        try {
            const day = parseInt(this.inputDay.current.value);
            if (isNaN(day)) throw "day is not a number!";
            const month = parseInt(this.inputMonth.current.value);
            if (isNaN(month)) throw "month is not a number!";
            const year = parseInt(this.inputYear.current.value);
            if (isNaN(year)) throw "year is not a number!";

            const date = new Date(year, month, day);
            if (date.getFullYear() !== year || date.getMonth() !== month || date.getDate() !== day) {
                throw "Invalid date!";
            }
            this.setState({ error: "" });

            const handler = this.props.onChange;
            if (typeof handler !== 'function') return;
            handler(date.getTime());
        }
        catch (ex) {
            this.setState({ error: _("invalid") });
        }
    }

    render() {
        const
            p = this.props,
            label = castString(p.label, ""),
            value = castInteger(p.value, 0),
            wide = castBoolean(p.wide, false),
            error = castString(this.state.error, ""),
            date = new Date(value),
            id = nextId();
        const classes = ["tfw-view-input-date", "thm-ele-button"];
        if (wide) classes.push("wide");
        const header = (error.length > 0 ? <div className="thm-bgSD error">{error}</div> :
            (label ? (<label htmlFor={id} className="thm-bgPD">{label}</label>) : null));
        return (<div className={classes.join(" ")} >
            {header}
            <div className="inputs">
                <input
                    ref={this.inputDay}
                    className="day"
                    type="number"
                    maxLength="2"
                    min="1"
                    max="31"
                    id={id}
                    size="2"
                    onChange={this.handleChange}
                    defaultValue={date.getDate()} />
                <select ref={this.inputMonth}
                    onChange={this.handleChange}
                    defaultValue={date.getMonth()}>{
                        [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11].map(monthId => (
                            <option key={monthId} value={monthId}>{_(`month-${monthId}`)}</option>
                        ))
                    }</select>
                <input
                    ref={this.inputYear}
                    className="year"
                    type="number"
                    maxLength="4"
                    size="4"
                    onChange={this.handleChange}
                    defaultValue={date.getFullYear()} />
            </div>
        </div>);
    }
}


let globalId = 0;
function nextId() {
    return `tfw-view-input-${globalId++}`;
}


const NUMBER_VALIDATOR = (value: string) => isNaN(parseFloat(value)) ? _('nan') : true;
