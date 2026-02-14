# neutron-backend-ts

Backend en TypeScript para el juego **Neutron**.

Incluye:
- API HTTP con Express.
- Comunicación en tiempo real con Socket.IO.
- Persistencia de estado en Redis.
- Registro de sesiones/eventos en PostgreSQL.
- Addons nativos C++ para minimax y RL (libtorch).

## Requisitos

- Node.js `>= 24`
- npm
- Redis
- PostgreSQL
- Toolchain nativo para addons:
  - `python3`
  - compilador C++ (`g++`/`clang++`)
  - `cmake`
  - `ninja`
- `libtorch` dentro del proyecto en `./libtorch`

## Configuración

1. Instala dependencias:

```bash
npm install
```

2. Crea tu archivo de entorno:

```bash
cp sample.env .env
```

3. Ajusta variables en `.env` según tu entorno.

Variables principales:
- `HOST` (default `0.0.0.0`)
- `PORT` (default `3000` en código, `3001` en `sample.env`)
- `CORS_ORIGINS` (coma-separado)
- `REDIS_URL`
- `PG_URL`
- `RL_MODEL_PATH` (default `data/model.pt`)

## Scripts

- Desarrollo:

```bash
npm run dev
```

- Build completo (TS + addons + assets):

```bash
npm run build
```

- Build addon minimax:

```bash
npm run build:native
```

- Build addon RL (usa `./libtorch` del proyecto):

```bash
npm run build:rl
```

- Producción (desde `dist`):

```bash
npm run start
```

## Salud del servicio

- `GET /health`: liveness (proceso activo)
- `GET /ready`: readiness (valida Redis y PostgreSQL)
  - `200` si todo OK
  - `503` si alguna dependencia no está lista

## Flujo de build y runtime nativo

### Compilación RL

El script `build:rl` fija explícitamente:
- `CMAKE_PREFIX_PATH=$PWD/libtorch`
- `Torch_DIR=$PWD/libtorch/share/cmake/Torch`

Con esto, `find_package(Torch REQUIRED)` toma el `libtorch` local del repo.

### Empaquetado a `dist`

`npm run copy-static-assets` copia:
- `libtorch/` a `dist/libtorch`
- `data/` a `dist/data`
- frontend estático a `dist/public`
- addons nativos a:
  - `dist/native/build/Release/neutron_minimax.node`
  - `dist/native/rl/build/Release/neutron_rl_addon.node`

## Troubleshooting

### Error: `Cannot find module '(src)/...'`
Asegúrate de arrancar con:

```bash
npm run start
```

Ese script precarga `module-alias/register`.

### Error Express 5 con wildcard `/*`
El proyecto usa patrón compatible con Express 5 (`/{*path}`) para fallback SPA.

### Error RL: `Could not find TorchConfig.cmake`
Verifica que exista:
- `./libtorch/share/cmake/Torch/TorchConfig.cmake`

Y vuelve a compilar:

```bash
rm -rf native/rl/build
npm run build:rl
```

### Error runtime RL: librerías compartidas faltantes
Revisa que `dist/libtorch/lib` exista y que hayas corrido:

```bash
npm run copy-static-assets
```

## Estructura relevante

- `src/server.ts`: bootstrap HTTP/WS, rutas y shutdown
- `src/game/engine.ts`: integración minimax/RL
- `src/game/store.ts`: persistencia Redis
- `src/infra/pg.ts`: pool de PostgreSQL
- `native/`: addon minimax
- `native/rl/`: addon RL con libtorch
