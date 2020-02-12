# Tide web interface

## Starting Tide server in development environment

```
npm run tide
```

This will spawn a local Tide server which mimic the real wall in level 0.
It will listen on port 8890.

The tide web UI is intended to be connected on 3 possible walls: 0 (local), 5 (real 5th floor) and 6 (Open Deck).

There is a proxy configuration in file `src/setupProxy.js` that makes the DEV environment to be like the PROD one.
