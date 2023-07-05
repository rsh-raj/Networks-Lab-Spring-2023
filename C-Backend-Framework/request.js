const axios = require('axios');

// // kuch toh ho rha hai
// axios
//       .get('http://127.0.0.1:8080/')
//       .then(
//         (response) => {
//           console.log(response.data);
//         }
//         ,
//         (error) => {
//           console.log(error);
//         }
//       );
axios.post('http://127.0.0.1:8080/about',
{
    "user_id" : "D2",
    "password" : "abc",
    "user" : null
}
    ,
    {
    headers: {
        "Content-Type": "application/json",
        "Connection":"keep-alive",
        "Iam" : "king"
    },
})
    .then(response => console.log(response.data))
    // .then(json1=> console.log(json1))
    .catch(err => console.error(err));

