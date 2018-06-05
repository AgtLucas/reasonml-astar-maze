open Cell;
open World;

module Grid = {
  let styles =
    Css.(
      {
        "grid": [
          display(flexBox),
          flexDirection(column),
          alignItems(stretch),
          justifyContent(center),
          width(vw(80.0)),
        ],
        "row": [
          display(flexBox),
          flexDirection(row),
          alignItems(stretch),
          justifyContent(center),
          width(pct(100.0)),
        ],
      }
    );

  let component = ReasonReact.statelessComponent("Grid");
  let make = (~matrix, ~onCellClick, _) => {
    ...component,
    render: _ => {
      let matrixNode =
        matrix
        |> Belt.Array.mapWithIndex(_, (y, row) =>
             Belt.Array.mapWithIndex(row, (x, type_) =>
               <Cell
                 onClick=(
                   _ =>
                     onCellClick(
                       (x, y),
                       switch (type_) {
                       | Wall => Empty
                       | Empty => Food
                       | Food => Wall
                       | _ => Empty
                       },
                     )
                 )
                 type_
               />
             )
           )
        |> Belt.Array.map(_, xs =>
             <div className=(Css.style(styles##row))>
               (ReasonReact.array(xs))
             </div>
           )
        |> ReasonReact.array;
      <div className=(Css.style(styles##grid))> matrixNode </div>;
    },
  };
};