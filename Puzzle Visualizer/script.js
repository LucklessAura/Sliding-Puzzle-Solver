
function checkBorder()
{
    var resultDiv = document.getElementById("output");
    if(!resultDiv.hasChildNodes())
    {
        resultDiv.style["padding"] = "0px"
    }
    else
    {
        resultDiv.style["padding"] = "7px"
    }
}



document.onload = checkBorder()


function DrawTable()
{
    var resultDiv = document.getElementById("output");
    var input = document.getElementById("inputField");
    var numbers = [];
    for (let i = 1; i < input.value.split("|").length -1; i++) {
       numbers.push(input.value.split("|")[i])
    }
    console.log(numbers);


    while (resultDiv.hasChildNodes()) {
        resultDiv.removeChild(resultDiv.firstChild);
    }
    var size = input.value.split("|")[0]

    for (let i = 0; i < size; i++) {
        var element = document.createElement("div");
        element.className = "row";
        for (let j = 0; j < size; j++) {
            var divEl = document.createElement("div");
            
            var text = document.createElement("p");
            divEl.appendChild(text);
            if( numbers[i * size + j ] === "-1")
            {
                divEl.id = "empty";
            }
            else
            {
                text.innerText = numbers[i * size + j ];
            }
            
            element.appendChild(divEl);
        }
        resultDiv.appendChild(element)
    }

    checkBorder();
    
}