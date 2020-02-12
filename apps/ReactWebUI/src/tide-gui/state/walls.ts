import { IWallInfo } from "../service/query"
import { IAction } from './types'

const PREFIX = "walls:"

export default {
    INITIAL_STATE: [],

    reducer(state: IWallInfo[], action: IAction): IWallInfo[] {
        const { type } = action;
        if (!type.startsWith(PREFIX)) return state;

        const command = action.type.substr(PREFIX.length);
        switch (command) {
            case "reset": return reset(action);
            default: throw Error(`Unknown action "${type}"!`);
        }
    },

    reset(walls: IWallInfo[]): IAction {
        return { type: `${PREFIX}reset`, walls };
    }
}


function reset(action: IAction): IWallInfo[] {
    return action.walls
}
