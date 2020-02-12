import React from "react"
import Button from "../button"

import "./input-file.css"

interface IInputFileProps {
    label?: string;
    icon?: string;
    wide?: boolean;
    wait?: boolean;
    flat?: boolean;
    small?: boolean;
    warning?: boolean;
    enabled?: boolean;
    multiple?: boolean;
    accept?: string;  // Ex.: "image/png,image/jpeg", ".png,.jpg", "image/*", ... See https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input/file#Unique_file_type_specifiers
    classes?: string[] | string;
    onClick?: (files: FileList) => void;
}

export default class InputFile extends React.Component<IInputFileProps, {}> {
    private readonly refInput: React.RefObject<HTMLInputElement> = React.createRef();

    constructor( props: IInputFileProps ) {
        super( props );
        this.handleClick = this.handleClick.bind( this );
        this.handleFileChange = this.handleFileChange.bind( this );
    }

    static readFileAsText(file: File): Promise<string> {
        return new Promise((resolve, reject) => {
            const reader: FileReader = new FileReader();
            reader.onload = (data) => {
                const content = data.target.result;
                resolve(content);
            };
            reader.onerror = (err) => {
                reject(err);
            };
            reader.readAsText(file);
        })
    }

    handleFileChange(evt: React.ChangeEvent<HTMLInputElement>) {
        if( !evt.target ) return;

        const handler = this.props.onClick;
        if (typeof handler !== 'function') return;

        try {
            const input = evt.target as HTMLInputElement;
            const files = input.files;
            if( !files ) return;
            handler(files);
        } catch(ex) {
            console.error("Error in handleFileChange(): ", );
            console.error(ex);
        }
    }

    handleClick() {
        const input = this.refInput.current;
        if( !input ) return;
        input.click();
    }

    render() {
        const p = this.props;
        const classes = ["tfw-view-InputFile"];
        if( p.wide === true ) classes.push("wide");

        return (<div className={classes.join(" ")}>
            <Button
                label={p.label}
                icon={p.icon}
                wide={p.wide}
                wait={p.wait}
                flat={p.flat}
                small={p.small}
                warning={p.warning}
                enabled={p.enabled}
                classes={p.classes}
                onClick={this.handleClick}/>
            <input
                ref={this.refInput}
                type="file"
                accept={p.accept}
                multiple={p.multiple === true}
                onChange={this.handleFileChange} />
        </div>)
    }
}
