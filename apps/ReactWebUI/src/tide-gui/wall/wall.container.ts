import { connect } from 'react-redux'
import { IAppState } from "../state"
import View from "./wall"

function mapStateToProps(state: IAppState) {
    return { walls: state.walls }
}

function mapDispatchToProps(dispatch) {
    return {
        // onClick: ...
    };
}

export default connect(mapStateToProps, mapDispatchToProps)(View);
