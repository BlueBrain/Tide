import { createStore } from 'redux'
import { IWallInfo } from "../service/query"
import { IAction } from './types'
import Walls from './walls'

export interface IAppState {
    walls: IWallInfo[]
}

const INITIAL_STATE: IAppState = {
    walls: Walls.INITIAL_STATE
};

function reducer(state: IAppState | undefined = INITIAL_STATE, action: IAction): IAppState {
    return {
        walls: Walls.reducer(state.walls, action)
    };
}

const store = createStore(reducer);
export default {
    store, dispatch: store.dispatch,
    Walls
};
