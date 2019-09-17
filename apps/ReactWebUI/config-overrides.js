const rewireYAML = require( 'react-app-rewire-yaml' );

module.exports = function override( config, env ) {
    // Allow YAML import at compile time.
    config = rewireYAML( config, env );
    return config;
}