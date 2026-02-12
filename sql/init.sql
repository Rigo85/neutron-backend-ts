CREATE TABLE IF NOT EXISTS sessions (
    id              BIGSERIAL PRIMARY KEY,
    socket_id       TEXT NOT NULL,
    ip              TEXT,
    user_agent      TEXT,
    accept_language TEXT,
    platform        TEXT,
    language        TEXT,
    screen_width    INT,
    screen_height   INT,
    color_depth     INT,
    timezone        TEXT,
    connected_at    TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    disconnected_at TIMESTAMPTZ
);

CREATE INDEX IF NOT EXISTS idx_sessions_socket_id ON sessions (socket_id);
CREATE INDEX IF NOT EXISTS idx_sessions_connected_at ON sessions (connected_at);

CREATE TABLE IF NOT EXISTS game_events (
    id          BIGSERIAL PRIMARY KEY,
    session_id  BIGINT REFERENCES sessions(id),
    game_id     TEXT,
    event       TEXT NOT NULL,
    payload     JSONB NOT NULL DEFAULT '{}',
    created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_game_events_game_id ON game_events (game_id);
CREATE INDEX IF NOT EXISTS idx_game_events_created_at ON game_events (created_at);
CREATE INDEX IF NOT EXISTS idx_game_events_game_over ON game_events (event, game_id) WHERE event = 'game_over';
