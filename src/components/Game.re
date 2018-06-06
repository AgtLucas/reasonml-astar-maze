open World;
open Grid;

let text = ReasonReact.string;

module Problem = SearchProblem.Make(PositionSearchProblem);
module GS = GraphSearch.Make(Problem);

let search = (~player=(0, 0), ~map) => {
  let world = World.make(map);
  let startState =
    PositionSearchProblem.getStartState({world, player, path: []});

  GS.search(startState, FoodAgent.heuristic)
  |> Belt.List.map(_, ({path}) =>
       World.fromState(
         ~path,
         ~world=startState.world,
         ~starting=startState.player,
       )
     );
};

let map2CellMatrix = (ls: array(array(int))) =>
  ls |> Belt.Array.map(_, Belt.Array.map(_, World.cellOfInt));

module Game = {
  let styles =
    Css.(
      {
        "game": [
          display(flexBox),
          flexDirection(column),
          alignItems(center),
          justifyContent(center),
          width(pct(100.)),
          height(pct(100.)),
        ],
        "searching": [color(hex("5764cc")), marginTop(vh(20.))],
        "title": [
          textTransform(uppercase),
          marginTop(px(50)),
          color(hex("E2E2E2")),
        ],
        "button": [
          selector(
            "& > button",
            [
              selector(
                "&:hover",
                [borderBottom(px(1), solid, hex("FFE400"))],
              ),
              selector("&[disabled]", [opacity(0.2), cursor(`default)]),
              borderWidth(px(0)),
              outlineStyle(none),
              borderBottom(px(1), solid, transparent),
              background(transparent),
              color(hex("FFE400")),
              paddingTop(px(12)),
              paddingBottom(px(2)),
              paddingRight(px(0)),
              paddingLeft(px(0)),
              textTransform(uppercase),
              margin(px(0)),
              cursor(`pointer),
            ],
          ),
          textAlign(center),
          width(px(100)),
        ],
        "controls": [
          display(flexBox),
          flexDirection(row),
          alignItems(center),
          justifyContent(spaceAround),
          marginBottom(px(16)),
          maxWidth(pct(80.)),
          width(px(400)),
        ],
      }
    );
  type action =
    | Rollback
    | Start
    | Edit
    | Search
    | ChangeCell(pos, cellT);

  type state = {
    steps: list(array(array(cellT))),
    map: array(array(int)),
    changed: bool,
    edit: bool,
    searching: bool,
  };
  let component = ReasonReact.reducerComponent("Game");

  let renderValue = (steps, value) =>
    <Grid
      editMode=false
      onCellClick=((_, _) => ())
      matrix=(
        Belt.List.getExn(
          steps,
          min(Belt.List.length(steps) - 1, int_of_float(value)),
        )
      )
    />;

  let remoteAction = RemoteAction.create();

  let make = _ => {
    ...component,
    initialState: () => {
      let map = Maps.allMaps[0];
      {steps: [], map, changed: true, edit: false, searching: false};
    },
    reducer: (action, state) =>
      switch (action) {
      | Edit => Update({...state, edit: ! state.edit})
      | ChangeCell((px, py), c) =>
        state.map[py][px] = intOfCell(c);
        Update({...state, map: state.map, changed: true});
      | Rollback =>
        SideEffects(
          (
            _ => RemoteAction.send(remoteAction, ~action=SpringComp.Start(0.))
          ),
        )
      | Search =>
        UpdateWithSideEffects(
          {...state, changed: false, searching: true},
          (
            ({send}) => {
              Js.Global.setTimeout(() => send(Start), 10) |> ignore;
              ();
            }
          ),
        )
      | Start =>
        UpdateWithSideEffects(
          {
            ...state,
            searching: false,
            steps: search(~player=(0, 0), ~map=state.map),
          },
          (
            ({state}) =>
              RemoteAction.send(
                remoteAction,
                ~action=
                  SpringComp.Start(
                    float_of_int(Belt.List.length(state.steps) - 1),
                  ),
              )
          ),
        )
      },
    didMount: ({send}) => send(Rollback),
    render: ({send, state: {map, steps, edit, changed, searching}}) =>
      <div className=(Css.style(styles##game))>
        <h1 className=(Css.style(styles##title))>
          (text("Maze - eat'em all"))
        </h1>
        <div className=(Css.style(styles##controls))>
          <div className=(Css.style(styles##button))>
            <button
              disabled=(edit || searching) onClick=(_ => send(Rollback))>
              (text("Rollback"))
            </button>
          </div>
          <div className=(Css.style(styles##button))>
            <button disabled=(edit || searching) onClick=(_ => send(Search))>
              (text("Search"))
            </button>
          </div>
          <div className=(Css.style(styles##button))>
            <button disabled=searching onClick=(_ => send(Edit))>
              (text(edit ? "Done" : "Edit"))
            </button>
          </div>
        </div>
        (
          switch (edit, changed, searching) {
          | (_, _, true) =>
            <div className=(Css.style(styles##searching))>
              (text("Searching..."))
            </div>
          | (true, _, _) =>
            <Grid
              editMode=edit
              matrix=(map2CellMatrix(map))
              onCellClick=((p, c) => send(ChangeCell(p, c)))
            />
          | (_, true, _) =>
            <Grid
              editMode=edit
              matrix=(map2CellMatrix(map))
              onCellClick=((p, c) => send(ChangeCell(p, c)))
            />
          | _ => <SpringComp remoteAction renderValue=(renderValue(steps)) />
          }
        )
      </div>,
  };
};